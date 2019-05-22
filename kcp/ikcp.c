//=====================================================================
//
// KCP - A Better ARQ Protocol Implementation
// skywind3000 (at) gmail.com, 2010-2011
//  
// Features:
// + Average RTT reduce 30% - 40% vs traditional ARQ like tcp.
// + Maximum RTT reduce three times vs tcp.
// + Lightweight, distributed as a single source file.
//
//=====================================================================
#include <log/cms_log.h>
//#include <net/gsdn_common.h>
#include <kcp/ikcp.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

//=====================================================================
// KCP BASIC
//=====================================================================
const IUINT32 IKCP_RTO_NDL = 10;		// no delay min rto
const IUINT32 IKCP_RTO_MIN = 100;		// normal min rto
const IUINT32 IKCP_RTO_DEF = 50;
const IUINT32 IKCP_RTO_MAX = 300;

const IUINT32 IKCP_ASK_SEND = 1;		// need to send IKCP_CMD_WASK
const IUINT32 IKCP_ASK_TELL = 2;		// need to send IKCP_CMD_WINS
const IUINT32 IKCP_WND_SND = 32;
const IUINT32 IKCP_WND_RCV = 32;
const IUINT32 IKCP_MTU_DEF = 1400;
const IUINT32 IKCP_ACK_FAST = 3;
const IUINT32 IKCP_INTERVAL = 100;
const IUINT32 IKCP_DEADLINK = 40;
const IUINT32 IKCP_THRESH_INIT = 2;
const IUINT32 IKCP_THRESH_MIN = 2;
const IUINT32 IKCP_PROBE_INIT = 100;		// at most 100ms to probe window size
const IUINT32 IKCP_PROBE_LIMIT = 5000;	// up to 5 secs to probe window


static const char* cmd_map[] = {
	"UNKOWN0",
	"PUSH",
	"ACK",
	"WASK",
	"WINS",
	"CLOSE",
	"CLOSE_CONFIRM",
	"CLOSE_ACK",
	"SYN",
	"RESET"
};

//---------------------------------------------------------------------
// encode / decode
//---------------------------------------------------------------------

///* encode 8 bits unsigned int */
//static inline char *ikcp_encode8u(char *p, unsigned char c)
//{
//	*(unsigned char*)p++ = c;
//	return p;
//}

///* decode 8 bits unsigned int */
//static inline const char *ikcp_decode8u(const char *p, unsigned char *c)
//{
//	*c = *(unsigned char*)p++;
//	return p;
//}

///* encode 16 bits unsigned int (lsb) */
//static inline char *ikcp_encode16u(char *p, unsigned short w)
//{
//#if IWORDS_BIG_ENDIAN
//	*(unsigned char*)(p + 0) = (w & 255);
//	*(unsigned char*)(p + 1) = (w >> 8);
//#else
//	*(unsigned short*)(p) = w;
//#endif
//	p += 2;
//	return p;
//}

///* decode 16 bits unsigned int (lsb) */
//static inline const char *ikcp_decode16u(const char *p, unsigned short *w)
//{
//#if IWORDS_BIG_ENDIAN
//	*w = *(const unsigned char*)(p + 1);
//	*w = *(const unsigned char*)(p + 0) + (*w << 8);
//#else
//	*w = *(const unsigned short*)p;
//#endif
//	p += 2;
//	return p;
//}

///* encode 32 bits unsigned int (lsb) */
//static inline char *ikcp_encode32u(char *p, IUINT32 l)
//{
//#if IWORDS_BIG_ENDIAN
//	*(unsigned char*)(p + 0) = (unsigned char)((l >>  0) & 0xff);
//	*(unsigned char*)(p + 1) = (unsigned char)((l >>  8) & 0xff);
//	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
//	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
//#else
//	*(IUINT32*)p = l;
//#endif
//	p += 4;
//	return p;
//}

///* decode 32 bits unsigned int (lsb) */
//static inline const char *ikcp_decode32u(const char *p, IUINT32 *l)
//{
//#if IWORDS_BIG_ENDIAN
//	*l = *(const unsigned char*)(p + 3);
//	*l = *(const unsigned char*)(p + 2) + (*l << 8);
//	*l = *(const unsigned char*)(p + 1) + (*l << 8);
//	*l = *(const unsigned char*)(p + 0) + (*l << 8);
//#else
//	*l = *(const IUINT32*)p;
//#endif
//	p += 4;
//	return p;
//}

static inline IUINT32 _imin_(IUINT32 a, IUINT32 b) {
	return a <= b ? a : b;
}

static inline IUINT32 _imax_(IUINT32 a, IUINT32 b) {
	return a >= b ? a : b;
}

static inline IUINT32 _ibound_(IUINT32 lower, IUINT32 middle, IUINT32 upper)
{
	return _imin_(_imax_(lower, middle), upper);
}

static inline long _itimediff(IUINT32 later, IUINT32 earlier)
{
	return ((IINT32)(later - earlier));
}

//---------------------------------------------------------------------
// manage segment
//---------------------------------------------------------------------
typedef struct IKCPSEG IKCPSEG;

static void* (*ikcp_malloc_hook)(size_t) = NULL;
static void(*ikcp_free_hook)(void *) = NULL;

// internal malloc
static void* ikcp_malloc(size_t size) {
	if (ikcp_malloc_hook)
		return ikcp_malloc_hook(size);
	return malloc(size);
}

// internal free
static void ikcp_free(void *ptr) {
	if (ikcp_free_hook) {
		ikcp_free_hook(ptr);
	}
	else {
		free(ptr);
	}
}

// redefine allocator
void ikcp_allocator(void* (*new_malloc)(size_t), void(*new_free)(void*))
{
	ikcp_malloc_hook = new_malloc;
	ikcp_free_hook = new_free;
}

// allocate a new kcp segment
static IKCPSEG* ikcp_segment_new(ikcpcb *kcp, int size)
{
	IKCPSEG* seg = (IKCPSEG*)ikcp_malloc(sizeof(IKCPSEG) + size);
	memset(seg, 0, sizeof(IKCPSEG));
	return seg;
}

// only copy data segment
static IKCPSEG* ikcp_segment_copy(ikcpcb *kcp, IKCPSEG* seg)
{
	IKCPSEG* seg2 = (IKCPSEG*)ikcp_malloc(sizeof(IKCPSEG) + seg->len);
	memcpy(seg2, seg, sizeof(IKCPSEG) + seg->len);
	return seg2;
}

// delete a segment
static void ikcp_segment_delete(ikcpcb *kcp, IKCPSEG *seg)
{
	ikcp_free(seg);
}

// write log
void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...)
{
	char buffer[1024];
	va_list argptr;
	if ((mask & kcp->logmask) == 0 || kcp->writelog == 0) return;
	va_start(argptr, fmt);
	vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	kcp->writelog(buffer, kcp, kcp->user);
}

// output segment
static int ikcp_output(ikcpcb *kcp, const void *data, int size)
{
	assert(kcp);
	assert(kcp->output);
	//	if (ikcp_canlog(kcp, IKCP_LOG_OUTPUT)) {
	//		ikcp_log(kcp, IKCP_LOG_OUTPUT, "[RO] %ld bytes", (long)size);
	//	}
	if (size == 0) return 0;
	return kcp->output((const char*)data, size, kcp, kcp->user);
}

// output queue
void ikcp_qprint(const char *name, const struct IQUEUEHEAD *head)
{
#if 0
	const struct IQUEUEHEAD *p;
	printf("<%s>: [", name);
	for (p = head->next; p != head; p = p->next) {
		const IKCPSEG *seg = iqueue_entry(p, const IKCPSEG, node);
		printf("(%lu %d)", (unsigned long)seg->sn, (int)(seg->ts % 10000));
		if (p->next != head) printf(",");
	}
	printf("]\n");
#endif
}


//---------------------------------------------------------------------
// create a new kcpcb
//---------------------------------------------------------------------
ikcpcb* ikcp_create(IUINT32 conv, int fd, int fdlisten, unsigned long ipInt, unsigned short port, void *user)
{
	ikcpcb *kcp = (ikcpcb*)ikcp_malloc(sizeof(struct IKCPCB));
	if (kcp == NULL) return NULL;
	memset(kcp, 0, sizeof(struct IKCPCB));

	kcp->fd = fd;
	kcp->fdlisten = fdlisten;
	kcp->maddr.sin_family = AF_INET;
	kcp->maddr.sin_port = htons(port);
	kcp->maddr.sin_addr.s_addr = ipInt;

	kcp->conv = conv;
	kcp->user = user;
	kcp->snd_una = 0;
	kcp->snd_nxt = 0;
	kcp->rcv_nxt = 0;
	kcp->ts_recent = 0;
	kcp->ts_lastack = 0;
	kcp->ts_probe = 0;
	kcp->probe_wait = 0;
	kcp->snd_wnd = IKCP_WND_SND;
	kcp->rcv_wnd = IKCP_WND_RCV;
	kcp->rmt_wnd = IKCP_WND_RCV;
	kcp->cwnd = 0;
	kcp->incr = 0;
	kcp->probe = 0;
	kcp->mtu = IKCP_MTU_DEF;
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;
	kcp->stream = 0; // don't use steam mode in udp !
	//kcp->timeout = 30;
	kcp->max_rto = 0;
	kcp->max_srtt = 0;

	kcp->buffer = (char*)ikcp_malloc((kcp->mtu + IKCP_OVERHEAD) * 3);
	if (kcp->buffer == NULL) {
		ikcp_free(kcp);
		return NULL;
	}

	iqueue_init(&kcp->snd_queue);
	iqueue_init(&kcp->rcv_queue);
	iqueue_init(&kcp->snd_buf);
	iqueue_init(&kcp->rcv_buf);
	kcp->nrcv_buf = 0;
	kcp->nsnd_buf = 0;
	kcp->nrcv_que = 0;
	kcp->nsnd_que = 0;
	kcp->state = 0;
	kcp->acklist = NULL;
	kcp->ackblock = 0;
	kcp->ackcount = 0;
	kcp->rx_srtt = 0;
	kcp->rx_rttval = 0;
	kcp->rx_rto = IKCP_RTO_DEF;
	kcp->rx_minrto = IKCP_RTO_MIN;
	kcp->current = 0;
	kcp->interval = IKCP_INTERVAL;
	kcp->ts_flush = IKCP_INTERVAL;
	kcp->nodelay = 0;
	kcp->updated = 0;
	kcp->logmask = 0;
	kcp->ssthresh = IKCP_THRESH_INIT;
	kcp->fastresend = 0;
	kcp->nocwnd = 0;
	kcp->dead_link = IKCP_DEADLINK;
	kcp->output = NULL;
	kcp->writelog = NULL;

	kcp->order = 1;

	kcp->resend_syn_t = 0;

	return kcp;
}


//---------------------------------------------------------------------
// release a new kcpcb
//---------------------------------------------------------------------
void ikcp_release(ikcpcb *kcp)
{
	assert(kcp);
	if (kcp) {
		IKCPSEG *seg;
		while (!iqueue_is_empty(&kcp->snd_buf)) {
			seg = iqueue_entry(kcp->snd_buf.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->rcv_buf)) {
			seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->snd_queue)) {
			seg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->rcv_queue)) {
			seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		if (kcp->buffer) {
			ikcp_free(kcp->buffer);
		}
		if (kcp->acklist) {
			ikcp_free(kcp->acklist);
		}

		kcp->nrcv_buf = 0;
		kcp->nsnd_buf = 0;
		kcp->nrcv_que = 0;
		kcp->nsnd_que = 0;
		kcp->ackcount = 0;
		kcp->buffer = NULL;
		kcp->acklist = NULL;
		ikcp_free(kcp);
	}
}


//---------------------------------------------------------------------
// set output callback, which will be invoked by kcp
//---------------------------------------------------------------------
void ikcp_setoutput(ikcpcb *kcp, int(*output)(const char *buf, int len,
	ikcpcb *kcp, void *user))
{
	kcp->output = output;
}


//---------------------------------------------------------------------
// user/upper level recv: returns size, returns below zero for EAGAIN
//---------------------------------------------------------------------
int ikcp_recv(ikcpcb *kcp, char *buffer, int len)
{
	struct IQUEUEHEAD *p, *next;
	int ispeek = (len < 0) ? 1 : 0;
	int peeksize;
	int recover = 0;
	IKCPSEG *seg;
	IKCPSEG *seg2;
	assert(kcp);

	if (iqueue_is_empty(&kcp->rcv_queue) && iqueue_is_empty(&kcp->rcv_buf))
	{
		if (kcp->close_fin == 1)//最后一个附带fin标记的数据包都已经处理完毕了 连接可以断开了
		{
			kcp->close_flag = 1;
		}
		if (kcp->close_flag == 1 || kcp->close_first || kcp->close_second || kcp->close_confirm || kcp->close_final == 1)//连接已经断开了
		{
			return IKCP_HAS_BEEN_EOF;
		}
		return -1;
	}


	if (len < 0) len = -len;

	peeksize = ikcp_peeksize(kcp);

	if (peeksize < 0)
		return -2;

	if (peeksize > len)
		return -3;

	if (kcp->nrcv_que >= kcp->rcv_wnd)
		recover = 1;

	// merge fragment
	for (len = 0, p = kcp->rcv_queue.next; p != &kcp->rcv_queue; ) {
		int fragment;
		seg = iqueue_entry(p, IKCPSEG, node);
		p = p->next;

		if (buffer) {
			memcpy(buffer, seg->data, seg->len);
			buffer += seg->len;
		}

		len += seg->len;
		fragment = seg->frg;

		//printf("merge sn:%d len:%d\n", seg->sn, seg->len);

		if (ispeek == 0) {
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
			kcp->nrcv_que--;
		}

		if (fragment == 0)
			break;
	}

	assert(len == peeksize);

	// move available data from rcv_buf -> rcv_queue
	// while( !iqueue_is_empty(&kcp->rcv_buf) ) {
	//		IKCPSEG *seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
	//		if ((seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd)) {
	//			iqueue_del(&seg->node);
	//			kcp->nrcv_buf--;
	//			iqueue_add_tail(&seg->node, &kcp->rcv_queue);
	//			kcp->nrcv_que++;
	//			kcp->rcv_nxt++;
	//		} else {
	//			break;
	//		}
	// }
	if (iqueue_is_empty(&kcp->rcv_buf)) {
		return len;
	}

	for (p = kcp->rcv_buf.next; p != &kcp->rcv_buf; p = next) {
		next = p->next;
		seg = iqueue_entry(p, IKCPSEG, node);
		if (kcp->nrcv_que >= kcp->rcv_wnd) {
			break;
		}

		if (seg->sn == kcp->rcv_nxt) {
			//			printf("conv:%u recv order sn:%d\n", kcp->conv, seg->sn);
			iqueue_del(&seg->node);
			kcp->nrcv_buf--;
			if (!seg->had_send) {
				iqueue_add_tail(&seg->node, &kcp->rcv_queue);
				kcp->nrcv_que++;
			}
			else {
				ikcp_segment_delete(kcp, seg);
			}
			kcp->rcv_nxt++;
		}
		else if (!kcp->order) {
			//			printf("conv:%u recv unorder sn:%d\n", kcp->conv, seg->sn);
			if (!seg->had_send) {
				seg->had_send = 1;
				seg2 = ikcp_segment_copy(kcp, seg);
				iqueue_init(&seg2->node);
				iqueue_add_tail(&seg2->node, &kcp->rcv_queue);
				kcp->nrcv_que++;
			}
		}
		else {
			break;
		}
	}
	// fast recover
	if (kcp->nrcv_que < kcp->rcv_wnd && recover) {
		// ready to send back IKCP_CMD_WINS in ikcp_flush
		// tell remote my window size
		kcp->probe |= IKCP_ASK_TELL;
	}

	return len;
}


//---------------------------------------------------------------------
// peek data size
//---------------------------------------------------------------------
int ikcp_peeksize(const ikcpcb *kcp)
{
	struct IQUEUEHEAD *p;
	IKCPSEG *seg;
	int length = 0;

	assert(kcp);

	if (iqueue_is_empty(&kcp->rcv_queue)) return -1;

	seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
	if (seg->frg == 0) return seg->len;

	if (kcp->nrcv_que < seg->frg + 1) return -1;

	for (p = kcp->rcv_queue.next; p != &kcp->rcv_queue; p = p->next) {
		seg = iqueue_entry(p, IKCPSEG, node);
		length += seg->len;
		if (seg->frg == 0) break;
	}

	return length;
}

// send syn package
int ikcp_send_syn(ikcpcb *kcp, const char *buffer, int len) {
	if (len < 0 || NULL == buffer)
	{
		logs->error("conv=%u ikcp_send_syn data error[%p:%d]", kcp->conv, buffer, len);
		return -1;
	}
	IKCPSEG *seg;
	seg = ikcp_segment_new(kcp, len);
	if (seg == NULL) {
		logs->error("conv=%u ikcp_send_syn ikcp_segment_new error[%p:%d]", kcp->conv, buffer, len);
		return -2;
	}
	seg->cmd = IKCP_CMD_SYN;
	seg->len = len;
	seg->frg = 0;
	memcpy(seg->data, buffer, len);
	iqueue_init(&seg->node);
	iqueue_add_tail(&seg->node, &kcp->snd_queue);
	kcp->nsnd_que++;
	logs->debug("conv=%u ikcp_send_syn add sync", kcp->conv);
	return 0;
}

void ikcp_close(ikcpcb *kcp) {
	kcp->close_first = 1;
}

int ikcp_rtm_rate(ikcpcb *kcp)
{
	int rate = 0;
	if (kcp->total_send > 0)
	{
		rate = kcp->total_resend * 100 / kcp->total_send;
	}
	//printf("kcp total_resend %lld,total_send %lld\n", kcp->total_resend, kcp->total_send);
	kcp->total_send = 0;
	kcp->total_resend = 0;
	return rate;
}

void ikcp_rtm_total(ikcpcb *kcp, IUINT64 *resend, IUINT64 *total)
{
	*resend = kcp->total_resend;
	*total = kcp->total_send;
}

int ikcp_mark_last_close(ikcpcb *kcp)
{
	struct IQUEUEHEAD *p, *next;
	IKCPSEG *markSeg = NULL;
	for (p = kcp->snd_queue.next; p != &kcp->snd_queue; p = next) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (seg->cmd == IKCP_CMD_PUSH || seg->cmd == IKCP_CMD_PUSH_AND_CLOSE)//IKCP_CMD_PUSH_AND_CLOSE 防止多次 调用该函数
		{
			markSeg = seg;
		}
	}
	if (markSeg != NULL)
	{
		markSeg->cmd = IKCP_CMD_PUSH_AND_CLOSE;
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------------
// user/upper level send, returns below zero for error
//---------------------------------------------------------------------
int ikcp_send(ikcpcb *kcp, const char *buffer, int len)
{
	if (kcp->close_final == 1 || kcp->close_first || kcp->close_second || kcp->close_confirm)//连接已经断开了 没有必要再发送数据了
	{
		return 0;
	}
	IKCPSEG *seg;
	int count, i;

	assert(kcp->mss > 0);
	if (len < 0) return -1;

	// append to previous segment in streaming mode (if possible)
	if (kcp->stream != 0) {
		//		if (!iqueue_is_empty(&kcp->snd_queue)) {
		//			IKCPSEG *old = iqueue_entry(kcp->snd_queue.prev, IKCPSEG, node);
		//			if (old->len < kcp->mss && old->cmd == IKCP_CMD_PUSH) {
		//				int capacity = kcp->mss - old->len;
		//				int extend = (len < capacity)? len : capacity;
		//				seg = ikcp_segment_new(kcp, old->len + extend);
		//				assert(seg);
		//				if (seg == NULL) {
		//					return -2;
		//				}
		//				iqueue_add_tail(&seg->node, &kcp->snd_queue);
		//				memcpy(seg->data, old->data, old->len);
		//				if (buffer) {
		//					memcpy(seg->data + old->len, buffer, extend);
		//					buffer += extend;
		//				}
		//				seg->len = old->len + extend;
		//				seg->frg = 0;
		//				seg->cmd = IKCP_CMD_PUSH;
		//				len -= extend;
		//				iqueue_del_init(&old->node);
		//				ikcp_segment_delete(kcp, old);
		//			}
		//		}
		//		if (len <= 0) {
		//			return 0;
		//		}
	}

	if (len <= (int)kcp->mss) count = 1;
	else count = (len + kcp->mss - 1) / kcp->mss;

	if (count > 255)
	{
		logs->warn("############################ kcp conv=%u count=%d too large &&&&&&&&&&&&&&&&&&&&&&&&", count);
		return -2;
	}

	if (count == 0) count = 1;

	// fragment
	for (i = 0; i < count; i++) {
		int size = len > (int)kcp->mss ? (int)kcp->mss : len;
		seg = ikcp_segment_new(kcp, size);
		assert(seg);
		if (seg == NULL) {
			return -2;
		}
		if (buffer && len > 0) {
			memcpy(seg->data, buffer, size);
		}
		seg->len = size;
		seg->cmd = IKCP_CMD_PUSH;
		seg->frg = (kcp->stream == 0) ? (count - i - 1) : 0;
		iqueue_init(&seg->node);
		iqueue_add_tail(&seg->node, &kcp->snd_queue);
		kcp->nsnd_que++;
		if (buffer) {
			buffer += size;
		}
		len -= size;
	}
	if (kcp->is_new_data <= 1)
	{
		kcp->is_new_data = 1;
	}
	return 0;
}


//---------------------------------------------------------------------
// parse ack
//---------------------------------------------------------------------
static void ikcp_update_ack(ikcpcb *kcp, IINT32 rtt)
{
	if (rtt > kcp->max_srtt)
	{
		kcp->max_srtt = rtt;
	}
	IINT32 rto = 0;
	if (kcp->rx_srtt == 0) {
		kcp->rx_srtt = rtt;
		kcp->rx_rttval = rtt / 2;
	}
	else {
		long delta = rtt - kcp->rx_srtt;
		if (delta < 0) delta = -delta;
		kcp->rx_rttval = (3 * kcp->rx_rttval + delta) / 4;
		kcp->rx_srtt = (7 * kcp->rx_srtt + rtt) / 8;
		if (kcp->rx_srtt < 1) kcp->rx_srtt = 1;
	}
	rto = kcp->rx_srtt + _imax_(kcp->interval, 4 * kcp->rx_rttval);
	kcp->rx_rto = _ibound_(kcp->rx_minrto, rto, IKCP_RTO_MAX);
	if (kcp->avg_ack_rtt == 0) {
		kcp->avg_ack_rtt = rtt;
	}
	else {
		kcp->avg_ack_rtt = (kcp->avg_ack_rtt + rtt) / 2;
	}
	if (rtt < 1000) {
		kcp->ack_rtt[rtt / 10]++;
	}
	else {
		kcp->ack_rtt[100]++;
	}	
}

static void ikcp_shrink_buf(ikcpcb *kcp)
{
	struct IQUEUEHEAD *p = kcp->snd_buf.next;
	if (p != &kcp->snd_buf) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		kcp->snd_una = seg->sn;
	}
	else {
		kcp->snd_una = kcp->snd_nxt;
	}
}

// 删除第sn个包，删除snd_buf 中的一个。
// 这样等snd_buf 删完，就发完了。snd_buf.next永远是要发的第一个包
static int ikcp_parse_ack(ikcpcb *kcp, IUINT32 sn)
{
	struct IQUEUEHEAD *p, *next;
	int ack_del = 0;
	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0)
		return 0;

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (sn == seg->sn) {
			if (seg->cmd == IKCP_CMD_SYN)
			{
				kcp->is_syn_succ = 1; //握手成功 允许发送数据
				logs->debug("0 conv=%u syn succ", kcp->conv);
			}
			iqueue_del(p);
			ack_del = 1;
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
			break;
		}
		if (_itimediff(sn, seg->sn) < 0) {
			break;
		}
	}
	return ack_del;
}

// 删除 una之前的所有的包
static void ikcp_parse_una(ikcpcb *kcp, IUINT32 una)
{
	struct IQUEUEHEAD *p, *next;
	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (_itimediff(una, seg->sn) > 0) {
			if (seg->cmd == IKCP_CMD_SYN)
			{
				kcp->is_syn_succ = 1; //握手成功 允许发送数据
				logs->debug("1 conv=%u syn succ", kcp->conv);
			}
			iqueue_del(p);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
		}
		else {
			break;
		}
	}
}

static void ikcp_parse_fastack(ikcpcb *kcp, IUINT32 sn)
{
	struct IQUEUEHEAD *p, *next;

	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0)
		return;

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (_itimediff(sn, seg->sn) < 0) {
			break;
		}
		else if (sn != seg->sn) {
			seg->fastack++;
			kcp->fastack_cnt++;
		}
	}
}


//---------------------------------------------------------------------
// ack append
//---------------------------------------------------------------------
static void ikcp_ack_push(ikcpcb *kcp, IUINT32 sn, IUINT32 ts)
{
	size_t newsize = kcp->ackcount + 1;
	IUINT32 *ptr;

	if (newsize > kcp->ackblock) {
		IUINT32 *acklist;
		size_t newblock;

		for (newblock = 8; newblock < newsize; newblock <<= 1);
		acklist = (IUINT32*)ikcp_malloc(newblock * sizeof(IUINT32) * 2);

		if (acklist == NULL) {
			assert(acklist != NULL);
			abort();
		}

		if (kcp->acklist != NULL) {
			size_t x;
			for (x = 0; x < kcp->ackcount; x++) {
				acklist[x * 2 + 0] = kcp->acklist[x * 2 + 0];
				acklist[x * 2 + 1] = kcp->acklist[x * 2 + 1];
			}
			ikcp_free(kcp->acklist);
		}

		kcp->acklist = acklist;
		kcp->ackblock = newblock;
	}

	ptr = &kcp->acklist[kcp->ackcount * 2];
	ptr[0] = sn;
	ptr[1] = ts;
	kcp->ackcount++;
}

static void ikcp_ack_get(const ikcpcb *kcp, int p, IUINT32 *sn, IUINT32 *ts)
{
	if (sn) sn[0] = kcp->acklist[p * 2 + 0];
	if (ts) ts[0] = kcp->acklist[p * 2 + 1];
}


//---------------------------------------------------------------------
// parse data
//---------------------------------------------------------------------
int ikcp_parse_data(ikcpcb *kcp, IKCPSEG *newseg)
{
	struct IQUEUEHEAD *p, *prev, *next;
	IKCPSEG *seg;
	IUINT32 sn = newseg->sn;
	int repeat = 0;

	int res = 0;
	// 数据包间隔太大
	if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) >= 0 ||
		// 数据包是之前ack过的
		_itimediff(sn, kcp->rcv_nxt) < 0) {
		ikcp_segment_delete(kcp, newseg);
		kcp->duplicate_packet++;
		return res;
	}
	res = 1;
	for (p = kcp->rcv_buf.prev; p != &kcp->rcv_buf; p = prev) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		prev = p->prev;
		if (seg->sn == sn) {
			repeat = 1;
			break;
		}
		if (_itimediff(sn, seg->sn) > 0) {
			break;
		}
	}

	if (repeat == 0) {
		iqueue_init(&newseg->node);
		iqueue_add(&newseg->node, p);
		kcp->nrcv_buf++;
	}
	else {
		kcp->repeat_packet++;
		ikcp_segment_delete(kcp, newseg);
	}

#if 0
	ikcp_qprint("rcvbuf", &kcp->rcv_buf);
	printf("rcv_nxt=%lu\n", kcp->rcv_nxt);
#endif

	// move available data from rcv_buf -> rcv_queue
	// while( !iqueue_is_empty(&kcp->rcv_buf) ) {
	//		IKCPSEG *seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
	//		if ((seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd)) {
	//			iqueue_del(&seg->node);
	//			kcp->nrcv_buf--;
	//			iqueue_add_tail(&seg->node, &kcp->rcv_queue);
	//			kcp->nrcv_que++;
	//			kcp->rcv_nxt++;
	//		} else {
	//			break;
	//		}
	// }
	if (iqueue_is_empty(&kcp->rcv_buf)) {
		return res;
	}
	for (p = kcp->rcv_buf.next; p != &kcp->rcv_buf; p = next) {
		next = p->next;
		seg = iqueue_entry(p, IKCPSEG, node);
		if (kcp->nrcv_que >= kcp->rcv_wnd) {
			break;
		}

		if (seg->sn == kcp->rcv_nxt) {
			//			printf("conv:%u recv order sn:%d\n", kcp->conv, seg->sn);
			iqueue_del(&seg->node);
			kcp->nrcv_buf--;
			if (!seg->had_send) {
				iqueue_add_tail(&seg->node, &kcp->rcv_queue);
				kcp->nrcv_que++;
			}
			else {
				ikcp_segment_delete(kcp, seg);
			}
			kcp->rcv_nxt++;
		}
		else if (!kcp->order) {
			//			printf("conv:%u recv unorder sn:%d\n", kcp->conv, seg->sn);
			if (!seg->had_send) {
				seg->had_send = 1;
				IKCPSEG *seg2 = ikcp_segment_copy(kcp, seg);
				iqueue_init(&seg2->node);
				iqueue_add_tail(&seg2->node, &kcp->rcv_queue);
				kcp->nrcv_que++;
			}
		}
		else {
			break;
		}
	}

#if 0
	ikcp_qprint("queue", &kcp->rcv_queue);
	printf("rcv_nxt=%lu\n", kcp->rcv_nxt);
#endif

#if 1
	//	printf("snd(buf=%d, queue=%d)\n", kcp->nsnd_buf, kcp->nsnd_que);
	//	printf("rcv(buf=%d, queue=%d)\n", kcp->nrcv_buf, kcp->nrcv_que);
#endif
	return res;
}


//---------------------------------------------------------------------
// input data
//---------------------------------------------------------------------

/*
data/syn : conv(4) cmd(1) frg(1) wnd(2) ts(4) sn(4) una(4) len(2) :22
other cmd: conv(4) cmd(1) wnd(2) ts(4) sn(4) una(4) : 19
*/
int ikcp_input(ikcpcb *kcp, const char *data, long size)
{
	IUINT32 una = kcp->snd_una;
	IUINT32 maxack = 0;
	int flag = 0;
	int ack_del = 0;

	if (data == NULL || (int)size < (int)IKCP_MINHEAD)
	{
		return -1;
	}

	while (size != 0) {
		IUINT32 ts, sn, una, conv;
		IUINT16 wnd, len;
		IUINT8 cmd, frg = 0;
		IKCPSEG *seg;

		data = ikcp_decode32u(data, &conv);
		if (conv != kcp->conv)
		{
			return -1;
		}

		data = ikcp_decode8u(data, &cmd);
		if (cmd < IKCP_CMD_BEG || cmd > IKCP_CMD_END)
		{
			return -3;
		}

		if (cmd == IKCP_CMD_SYN || cmd == IKCP_CMD_PUSH || cmd == IKCP_CMD_PUSH_AND_CLOSE) {
			if (size < (int)IKCP_OVERHEAD) break;
		}
		else {
			if (size < (int)IKCP_MINHEAD) break;
		}

		if (cmd == IKCP_CMD_SYN || cmd == IKCP_CMD_PUSH || cmd == IKCP_CMD_PUSH_AND_CLOSE) {
			data = ikcp_decode8u(data, &frg);
			data = ikcp_decode16u(data, &wnd);
			data = ikcp_decode32u(data, &ts);
			data = ikcp_decode32u(data, &sn);
			data = ikcp_decode32u(data, &una);
			data = ikcp_decode16u(data, &len);

			size -= IKCP_OVERHEAD;

			if ((long)size < (long)len) {
				return -2;
			}
		}
		else {
			data = ikcp_decode16u(data, &wnd);
			data = ikcp_decode32u(data, &ts);
			data = ikcp_decode32u(data, &sn);
			data = ikcp_decode32u(data, &una);

			len = 0;
			size -= IKCP_MINHEAD;
		}

		kcp->rmt_wnd = wnd;
		ikcp_parse_una(kcp, una);
		ikcp_shrink_buf(kcp);
		//printf("recv_conv:%u cmd:%u, sn:%d, snd_sn:%d, rcv_sn:%d frg:%d len:%d \n", conv, cmd, sn, kcp->snd_nxt, kcp->rcv_nxt, frg, len);
		if (cmd == IKCP_CMD_ACK) {

			if (_itimediff(kcp->current, ts) >= 0) {
				ikcp_update_ack(kcp, _itimediff(kcp->current, ts));
			}
			ack_del = ikcp_parse_ack(kcp, sn);
			ikcp_shrink_buf(kcp);
			if (ack_del) {
				if (flag == 0) {
					flag = 1;
					maxack = sn;
				}
				else {
					if (_itimediff(sn, maxack) > 0) {
						maxack = sn;
					}
				}
			}
			kcp->max_ack = maxack;
			//			if (ikcp_canlog(kcp, IKCP_LOG_IN_ACK)) {
			//				ikcp_log(kcp, IKCP_LOG_IN_DATA,
			//					"input ack: sn=%lu rtt=%ld rto=%ld", sn,
			//					(long)_itimediff(kcp->current, ts),
			//					(long)kcp->rx_rto);
			//			}
		}
		else if (cmd == IKCP_CMD_PUSH || cmd == IKCP_CMD_PUSH_AND_CLOSE) {
			if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) < 0) {
				int is_save = 0;
				ikcp_ack_push(kcp, sn, ts);
				if (_itimediff(sn, kcp->rcv_nxt) >= 0) {
					seg = ikcp_segment_new(kcp, len);
					seg->conv = conv;
					seg->cmd = cmd;
					seg->frg = frg;
					seg->wnd = wnd;
					seg->ts = ts;
					seg->sn = sn;
					seg->una = una;
					seg->len = len;

					if (len > 0) {
						memcpy(seg->data, data, len);
					}

					is_save = ikcp_parse_data(kcp, seg);
				}
				if (cmd == IKCP_CMD_PUSH_AND_CLOSE && is_save)
				{
					kcp->close_fin = 1; //标记收到fin包
				}
			}
		}
		else if (cmd == IKCP_CMD_WASK) {
			// ready to send back IKCP_CMD_WINS in ikcp_flush
			// tell remote my window size
			kcp->probe |= IKCP_ASK_TELL;
			//			if (ikcp_canlog(kcp, IKCP_LOG_IN_PROBE)) {
			//				ikcp_log(kcp, IKCP_LOG_IN_PROBE, "input probe");
			//			}
		}
		else if (cmd == IKCP_CMD_WINS) {
			// do nothing
//			if (ikcp_canlog(kcp, IKCP_LOG_IN_WINS)) {
//				ikcp_log(kcp, IKCP_LOG_IN_WINS,
//					"input wins: %lu", (IUINT32)(wnd));
//			}
		}
		else if (cmd == IKCP_CMD_SYN) {
// 			if ((int)len < 0) {
// 				return -4;
// 			}
			kcp->is_syn_succ = 1;
			ikcp_ack_push(kcp, sn, ts);
			logs->debug("conv=%u recv syn", kcp->conv);
			if (!kcp->syn) {
				kcp->syn = 1;
				kcp->rcv_nxt++;
			}
			//ikcp_force_update(kcp, iclock()); //强制发送响应 快速握手 目前有问题 by water
		}
		else if (cmd == IKCP_CMD_CLOSE) {
			//if (kcp->nsnd_buf == 0 && kcp->nsnd_que == 0) {
				// kcp is half closed.
//				ikcp_ack_push(kcp, sn, ts);
			kcp->close_second = 1;
			if (kcp->close_first == 1) {
				kcp->close_confirm = 1;
				//					printf("send 85, recv 85\n");
			}
			else {
				//					printf("only recv 85, kcp close first : %d\n", kcp->close_first);
			}
			//}
		}
		else if (cmd == IKCP_CMD_CLOSE_CONFIRM) {
			//			ikcp_ack_push(kcp, sn, ts);
			//			printf("ikcp recv confirm\n");
						// kcp is totally closed.
			if (kcp->close_confirm == 1) {
				kcp->close_final = 1;
				kcp->state = IKCP_STATE_EOF;
				//				printf("confirm final.\n");
			}
			else {
				kcp->close_confirm = 1;
				//				printf("recv confirm.\n");
			}
		}
		else if (cmd == IKCP_CMD_RESET) {
			++kcp->reset_cnt;
			if (kcp->reset_cnt > 5) {
				kcp->close_final = 1;
				kcp->state = -IKCP_STATE_RESET;
			}
		}
		else {
			return -3;
		}

		if (cmd != IKCP_CMD_RESET) {
			kcp->reset_cnt = 0;
		}

		data += len;
		size -= len;
	}

	if (flag != 0) {
		ikcp_parse_fastack(kcp, maxack);
	}

	if (_itimediff(kcp->snd_una, una) > 0) {
		if (kcp->cwnd < kcp->rmt_wnd) {
			IUINT32 mss = kcp->mss;
			if (kcp->cwnd < kcp->ssthresh) {
				kcp->cwnd++;
				kcp->incr += mss;
			}
			else {
				if (kcp->incr < mss) kcp->incr = mss;
				kcp->incr += (mss * mss) / kcp->incr + (mss / 16);
				if ((kcp->cwnd + 1) * mss <= kcp->incr) {
					kcp->cwnd++;
				}
			}
			if (kcp->cwnd > kcp->rmt_wnd) {
				kcp->cwnd = kcp->rmt_wnd;
				kcp->incr = kcp->rmt_wnd * mss;
			}
		}
	}

	kcp->last_input_ts = kcp->current;
	return 0;
}


//---------------------------------------------------------------------
// ikcp_encode_seg
//---------------------------------------------------------------------
static char *ikcp_encode_seg(char *ptr, const IKCPSEG *seg)
{
	ptr = ikcp_encode32u(ptr, seg->conv);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->cmd);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->frg);
	ptr = ikcp_encode16u(ptr, (IUINT16)seg->wnd);
	ptr = ikcp_encode32u(ptr, seg->ts);
	ptr = ikcp_encode32u(ptr, seg->sn);
	ptr = ikcp_encode32u(ptr, seg->una);
	ptr = ikcp_encode16u(ptr, seg->len);
	return ptr;
}

static char *ikcp_encode_seg_cmd(char *ptr, const kcpseg_cmd *seg)
{
	ptr = ikcp_encode32u(ptr, seg->conv);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->cmd);
	ptr = ikcp_encode16u(ptr, (IUINT16)seg->wnd);
	ptr = ikcp_encode32u(ptr, seg->ts);
	ptr = ikcp_encode32u(ptr, seg->sn);
	ptr = ikcp_encode32u(ptr, seg->una);
	return ptr;
}

static int ikcp_wnd_unused(const ikcpcb *kcp)
{
	if (kcp->nrcv_que < kcp->rcv_wnd) {
		return kcp->rcv_wnd - kcp->nrcv_que;
	}
	return 0;
}

//---------------------------------------------------------------------
// ikcp_flush
//---------------------------------------------------------------------
int ikcp_flush(ikcpcb *kcp, int ackOnly)
{
	int ret = 0;
	IUINT32 current = kcp->current;
	char *buffer = kcp->buffer;
	char *ptr = buffer;
	int count, size, i;
	IUINT32 cwnd;
	//	IUINT32 resent;
	IUINT32 rtomin;
	struct IQUEUEHEAD *p;
	//	int change = 0;
	int lost = 0;
	IKCPSEG seg;
	kcpseg_cmd seg_cmd;

	// 'ikcp_update' haven't been called. 
	if (kcp->updated == 0)
	{
		return ret;
	}

	if (kcp->last_input_ts == 0) {
		kcp->last_input_ts = current;
	}
	// 	else if (_itimediff(current, kcp->last_input_ts) > (kcp->timeout * 1000)) {
	// 		kcp->state = -IKCP_STATE_TIMEOUT;
	// 		kcp->close_final = 1;
	// 		return ret;
	// 	}
	seg.conv = kcp->conv;
	seg.cmd = IKCP_CMD_ACK;
	seg.frg = 0;
	seg.wnd = ikcp_wnd_unused(kcp);
	seg.una = kcp->rcv_nxt;
	seg.len = 0;
	seg.sn = 0;
	seg.ts = 0;

	memset(&seg_cmd, 0, sizeof(seg_cmd));
	seg_cmd.conv = kcp->conv;
	seg_cmd.cmd = IKCP_CMD_ACK;
	seg_cmd.wnd = ikcp_wnd_unused(kcp);
	seg_cmd.una = kcp->rcv_nxt;

	// flush acknowledges
	count = kcp->ackcount;
	for (i = 0; i < count; i++) {
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_MINHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ikcp_ack_get(kcp, i, &seg_cmd.sn, &seg_cmd.ts);
		ptr = ikcp_encode_seg_cmd(ptr, &seg_cmd);
	}

	kcp->ackcount = 0;

	if (ackOnly == 1)//只发送ack
	{
		// flash remain segments
		size = (int)(ptr - buffer);
		if (size > 0) {
			ikcp_output(kcp, buffer, size);
			ret = 1;
		}
		return ret;
	}

	// probe window size (if remote window size equals zero)
	if (kcp->rmt_wnd == 0) {
		if (kcp->probe_wait == 0) {
			//kcp->probe_wait = IKCP_PROBE_INIT;
			kcp->probe_wait = kcp->rx_rto * 10;
			if (kcp->probe_wait < IKCP_PROBE_INIT) {
				kcp->probe_wait = IKCP_PROBE_INIT;
			}
			kcp->ts_probe = kcp->current + kcp->probe_wait;
		}
		else {
			if (_itimediff(kcp->current, kcp->ts_probe) >= 0) {
				if (kcp->probe_wait < ((IUINT32)kcp->rx_rto * 10))
					kcp->probe_wait = kcp->rx_rto * 10;;
				kcp->probe_wait += kcp->probe_wait / 2;
				if (kcp->probe_wait > IKCP_PROBE_LIMIT)
					kcp->probe_wait = IKCP_PROBE_LIMIT;
				kcp->ts_probe = kcp->current + kcp->probe_wait;
				kcp->probe |= IKCP_ASK_SEND;
			}
		}
	}
	else {
		kcp->ts_probe = 0;
		kcp->probe_wait = 0;
	}

	// flush window probing commands
	if (kcp->probe & IKCP_ASK_SEND) {
		seg_cmd.cmd = IKCP_CMD_WASK;
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_MINHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg_cmd(ptr, &seg_cmd);
		//		printf("send probe ask\n");
	}

	// flush window probing commands
	if (kcp->probe & IKCP_ASK_TELL) {
		seg_cmd.cmd = IKCP_CMD_WINS;
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_MINHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg_cmd(ptr, &seg_cmd);
	}

	// flush window close commands
	if (kcp->close_confirm != 1 && (kcp->close_first == 1 || kcp->close_second == 1))
	{
		//printf("before close nsnd_buf:%u nsnd_que:%u kcp->rx_rto:%u\n", kcp->nsnd_buf, kcp->nsnd_que);
		if ((kcp->nsnd_buf == 0 && kcp->nsnd_que == 0) && (kcp->close_ts == 0 || _itimediff(current, kcp->close_ts) > kcp->rx_rto * 4)) {
			//printf("send close 85\n");
			if (kcp->send_close_time++ > 10) {
				kcp->state = -IKCP_STATE_CLOSE_TIMEOUT;
				kcp->close_final = 1;
			}
			kcp->close_ts = current;
			seg_cmd.cmd = IKCP_CMD_CLOSE;
			size = (int)(ptr - buffer);
			if (size + (int)IKCP_MINHEAD > (int)kcp->mtu) {
				ikcp_output(kcp, buffer, size);
				ptr = buffer;
			}
			ptr = ikcp_encode_seg_cmd(ptr, &seg_cmd);
		}
	}

	// flush close confirm command
	if (kcp->close_confirm == 1)
	{
		if ((kcp->nsnd_buf == 0) && (kcp->close_confirm_ts == 0 || _itimediff(current, kcp->close_confirm_ts) > kcp->rx_rto * 4)) {
			//			printf("send close 86 %u \n", iclock());
			seg_cmd.cmd = IKCP_CMD_CLOSE_CONFIRM;
			size = (int)(ptr - buffer);
			if (size + (int)IKCP_MINHEAD > (int)kcp->mtu) {
				ikcp_output(kcp, buffer, size);
				ptr = buffer;
			}
			ptr = ikcp_encode_seg_cmd(ptr, &seg_cmd);
			kcp->close_confirm_ts = current;
			if (kcp->close_confirm_cnt++ > 5) {
				kcp->state = IKCP_STATE_EOF;
				kcp->close_final = 1;
			}
		}
	}

	kcp->probe = 0;

	// calculate window size
	cwnd = _imin_(kcp->snd_wnd, kcp->rmt_wnd);



	if (kcp->nocwnd == 0) cwnd = _imin_(kcp->cwnd, cwnd);
	//	printf("%lu nocwnd:%d nrcv_que:%d nrcv_buf:%d nsnd_que:%d nsnd_buf:%d snd_nxt:%d snd_una:%d kcp_cwnd:%d cwnd:%d rmt_wnd:%d xmit:%d xmit1:%d xmit2:%d max_rexmit:%d rx_rto:%d \n",
	//		   iclock64(),kcp->nrcv_que, kcp->nrcv_buf, kcp->nocwnd, kcp->nsnd_que, kcp->nsnd_buf, kcp->snd_nxt, kcp->snd_una, kcp->cwnd, cwnd, kcp->rmt_wnd, kcp->xmit, kcp->xmit1, kcp->xmit2, kcp->max_rexmit, kcp->rx_rto);

		// move data from snd_queue to snd_buf
	while (_itimediff(kcp->snd_nxt, kcp->snd_una + cwnd) < 0) {
		IKCPSEG *newseg;
		if (iqueue_is_empty(&kcp->snd_queue)) break;

		newseg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);
		if (kcp->is_syn_succ == 0 && newseg->cmd != IKCP_CMD_SYN)
		{
			//logs->debug("conv=%u syn not succ, can not send data", kcp->conv);
			break; //如果握手没有成功 只允许发握手包 如果同时发送 有可能其它数据包先到 导致对方发送IKCP_CMD_RESET
		}
		iqueue_del(&newseg->node);
		iqueue_add_tail(&newseg->node, &kcp->snd_buf);
		kcp->nsnd_que--;
		kcp->nsnd_buf++;

		newseg->conv = kcp->conv;
		//		newseg->cmd = IKCP_CMD_PUSH;
		newseg->wnd = seg.wnd;
		newseg->ts = current;
		newseg->sn = kcp->snd_nxt++;
		newseg->una = kcp->rcv_nxt;
		newseg->resendts = current;
		newseg->rto = kcp->rx_rto;
		newseg->fastack = 0;
		newseg->xmit = 0;
		newseg->xmit2 = 0;
		newseg->had_send = 0;
	}

	// calculate resent
//	resent = (kcp->fastresend > 0)? (IUINT32)kcp->fastresend : 0xffffffff;
	rtomin = (kcp->nodelay == 0) ? (kcp->rx_rto >> 3) : 0;

	if (kcp->rx_rto > 50 && kcp->fix_rto > 0)
		kcp->rx_rto = kcp->fix_rto;

	// flush data segments
	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) {
		IKCPSEG *segment = iqueue_entry(p, IKCPSEG, node);
		int needsend = 0;
		if (segment->xmit == 0) {
			needsend = 1;
			kcp->xmit++;
			kcp->packet_cnt++;
			segment->xmit++;
			segment->rto = kcp->rx_rto;
			segment->resendts = current + segment->rto + rtomin;
		}
		else if (_itimediff(current, segment->resendts) >= 0) {
			needsend = 1;
			segment->xmit++;
			kcp->xmit1++;
			kcp->packet_cnt++;
			kcp->rexmit_cnt++;
			if (kcp->max_rexmit < segment->xmit) {
				kcp->max_rexmit = segment->xmit;
			}
			if (kcp->nodelay == 0) {
				segment->rto += kcp->rx_rto;
			}
			else {
				segment->rto += kcp->rx_rto / 2;
			}
			segment->resendts = current + segment->rto;

			lost = 1;
		}
		else if (kcp->fastresend && kcp->nsnd_buf > 300 && p == kcp->snd_buf.next &&
			(_itimediff(segment->resendts, current) >= kcp->rx_rto)) {
			needsend = 1;
			segment->xmit++;
			kcp->xmit2++;
			kcp->packet_cnt++;
			kcp->rexmit_cnt++;
			if (kcp->max_rexmit < segment->xmit) {
				kcp->max_rexmit = segment->xmit;
			}
			segment->resendts = current + kcp->rx_rto;
			lost = 1;
		}
		//		else if (segment->fastack >= resent) {
		//			needsend = 1;
		//			kcp->xmit2++;
		//			kcp->packet_cnt++;
		//			kcp->rexmit_cnt++;
		//			segment->xmit2++;
		//			segment->fastack = 0;
		//			segment->resendts = current + segment->rto;
		//			change++;
		//		}		
		if (needsend) {
			if (segment->xmit == 1)
			{
				kcp->total_send++;
			}
			else if (segment->xmit >= 2)
			{
				kcp->total_resend++;
			}
			if (segment->cmd == IKCP_CMD_SYN && lost)
			{
				if (current-kcp->resend_syn_t > 5000)
				{
					logs->debug("conv=%u resend syn", kcp->conv);
					kcp->resend_syn_t = current;
				}				
			}
			int size, need;
			segment->ts = current;
			segment->wnd = seg.wnd;
			segment->una = kcp->rcv_nxt;

			size = (int)(ptr - buffer);
			need = IKCP_OVERHEAD + segment->len;
			//printf("output sn %u len %u ts %u cmd %d \n", segment->sn, segment->len, segment->ts, segment->cmd);
			if (size + need > (int)kcp->mtu) {
				ikcp_output(kcp, buffer, size);
				ptr = buffer;
				ret = 1;
			}

			ptr = ikcp_encode_seg(ptr, segment);

			if (segment->len > 0) {
				memcpy(ptr, segment->data, segment->len);
				ptr += segment->len;
			}
			//			if (segment->xmit2 > 0) {
			//				kcp->state = segment->xmit2;
			//			}
			if (segment->xmit >= kcp->dead_link) {
				kcp->state = segment->xmit;
				kcp->close_final = 1;
				break;
			}
		}
	}

	// flash remain segments
	size = (int)(ptr - buffer);
	if (size > 0) {
		ikcp_output(kcp, buffer, size);
		ret = 1;
	}

	// update ssthresh
//	if (change) {
	if (1) {
		IUINT32 inflight = kcp->snd_nxt - kcp->snd_una;
		kcp->ssthresh = inflight / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN)
			kcp->ssthresh = IKCP_THRESH_MIN;
		//		kcp->cwnd = kcp->ssthresh + resent;

		if (kcp->had_loss > 1 && kcp->rx_rto > 100) {
			kcp->cwnd = kcp->cwnd / 2;
		}
		else {
			kcp->cwnd = kcp->rmt_wnd - inflight;
		}
		kcp->incr = kcp->cwnd * kcp->mss;
	}

	if (lost) {
		kcp->ssthresh = cwnd / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN)
			kcp->ssthresh = IKCP_THRESH_MIN;
		if (kcp->snd_una > 5) {
			kcp->had_loss++;
		}
		kcp->incr = kcp->mss;
	}
	else {
		if (kcp->had_loss > 0) {
			kcp->had_loss--;
		}
	}

	if (kcp->cwnd < 5) {
		kcp->cwnd = 5;
		kcp->incr = kcp->mss;
	}
	return ret;
}


//---------------------------------------------------------------------
// update state (call it repeatedly, every 10ms-100ms), or you can ask 
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec. 
//---------------------------------------------------------------------
int ikcp_update(ikcpcb *kcp, IUINT32 current)
{
	int ret = 0;
	IINT32 slap;

	kcp->current = current;

	if (kcp->updated == 0) {
		kcp->updated = 1;
		kcp->ts_flush = kcp->current;
	}

	slap = _itimediff(kcp->current, kcp->ts_flush);

	if (slap >= 10000 || slap < -10000) {
		kcp->ts_flush = kcp->current;
		slap = 0;
	}
	if (kcp->is_new_data == 1)
	{
		ret = ikcp_flush(kcp, 0);
		kcp->is_new_data = 2;
	}
	else if (slap >= 0) {
		kcp->ts_flush += kcp->interval;
		if (_itimediff(kcp->current, kcp->ts_flush) >= 0) {
			kcp->ts_flush = kcp->current + kcp->interval;
		}
		ret = ikcp_flush(kcp, 0);
	}
	return ret;
}

int ikcp_force_update(ikcpcb *kcp, IUINT32 current)
{
	kcp->current = current;
	kcp->ts_flush = kcp->current + kcp->interval;
	return ikcp_flush(kcp, 0);
}

//---------------------------------------------------------------------
// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there 
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to 
// schedule ikcp_update (eg. implementing an epoll-like mechanism, 
// or optimize ikcp_update when handling massive kcp connections)
//---------------------------------------------------------------------
IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current)
{
	IUINT32 ts_flush = kcp->ts_flush;
	IINT32 tm_flush = 0x7fffffff;
	IINT32 tm_packet = 0x7fffffff;
	IUINT32 minimal = 0;
	struct IQUEUEHEAD *p;

	if (kcp->updated == 0) {
		return current;
	}

	if (_itimediff(current, ts_flush) >= 10000 ||
		_itimediff(current, ts_flush) < -10000) {
		ts_flush = current;
	}

	if (_itimediff(current, ts_flush) >= 0) {
		return current;
	}

	tm_flush = _itimediff(ts_flush, current);

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) {
		const IKCPSEG *seg = iqueue_entry(p, const IKCPSEG, node);
		IINT32 diff = _itimediff(seg->resendts, current);
		if (diff <= 0) {
			return current;
		}
		if (diff < tm_packet) tm_packet = diff;
	}

	minimal = (IUINT32)(tm_packet < tm_flush ? tm_packet : tm_flush);
	if (minimal >= kcp->interval) minimal = kcp->interval;

	return current + minimal;
}



int ikcp_setmtu(ikcpcb *kcp, int mtu)
{
	char *buffer;
	if (mtu < 50 || mtu < (int)IKCP_OVERHEAD)
		return -1;
	buffer = (char*)ikcp_malloc((mtu + IKCP_OVERHEAD) * 3);
	if (buffer == NULL)
		return -2;
	kcp->mtu = mtu;
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;
	ikcp_free(kcp->buffer);
	kcp->buffer = buffer;
	return 0;
}

int ikcp_interval(ikcpcb *kcp, int interval)
{
	if (interval > 5000) interval = 5000;
	else if (interval < 10) interval = 10;
	kcp->interval = interval;
	return 0;
}

int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc)
{
	if (nodelay >= 0) {
		kcp->nodelay = nodelay;
		if (nodelay) {
			kcp->rx_minrto = IKCP_RTO_NDL;
		}
		else {
			kcp->rx_minrto = IKCP_RTO_MIN;
		}
	}
	if (interval >= 0) {
		if (interval > 5000) interval = 5000;
		else if (interval < 5) interval = 5;
		kcp->interval = interval;
	}
	if (resend >= 0) {
		kcp->fastresend = resend;
	}
	if (nc >= 0) {
		kcp->nocwnd = nc;
	}
	return 0;
}


int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd)
{
	if (kcp) {
		if (sndwnd > 0) {
			kcp->snd_wnd = sndwnd;
		}
		if (rcvwnd > 0) {
			kcp->rcv_wnd = rcvwnd;
			kcp->cwnd = rcvwnd;// send the most data at start
		}
	}
	return 0;
}

int ikcp_waitsnd(const ikcpcb *kcp)
{
	return kcp->nsnd_buf + kcp->nsnd_que;
}

int ikcp_snd_full(ikcpcb *kcp)
{
	if (kcp->nsnd_que + kcp->nsnd_buf > 2 * kcp->snd_wnd)
	{
		return 1;
	}
	return 0;
}


// read conv
IUINT32 ikcp_getconv(const void *ptr)
{
	IUINT32 conv;
	ikcp_decode32u((const char*)ptr, &conv);
	return conv;
}



void ikcp_reorder(ikcpcb *kcp, IUINT8 order)
{
	kcp->order = order;
}

int ikcp_get_reset_data(IUINT32 conv, char *buff)
{
	kcpseg_cmd seg;
	memset(&seg, 0, sizeof(kcpseg_cmd));
	seg.cmd = IKCP_CMD_RESET;
	seg.conv = conv;
	ikcp_encode_seg_cmd(buff, &seg);
	return IKCP_MINHEAD;
}

const char *ikcp_get_cmd_str(IUINT32 cmd)
{
	if (cmd <= IKCP_CMD_BEG || cmd >= IKCP_CMD_END) {
		return "UNKOWN_CMD";
	}
	return cmd_map[cmd - IKCP_CMD_BEG];
}

void ikcp_get_srtt(ikcpcb *kcp, int *max_srtt, int *rx_srtt, int *rx_rttval)
{
	*max_srtt = (int)kcp->max_srtt;
	kcp->max_srtt = 0;
	*rx_srtt = (int)kcp->rx_srtt;
	*rx_rttval = (int)kcp->rx_rttval;
}

