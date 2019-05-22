rm -rf *.o libH264_5.a
g++ -c BitStream.cpp ParseH264Header.cpp  ParseHevcHeader.cpp ParseSequenceHeader.cpp decode.cpp
ar -cr libH264_5.a *.o
rm -rf *.o

