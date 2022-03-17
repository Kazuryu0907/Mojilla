#include "UTFCheck.hpp"

std::vector<std::string> UTFCheck::get(std::string str){
    {
		for (const auto& item : str) {
			std::stringstream ss;
			ss << std::hex << int(item);
			if(ss.str().find("ffffff") != std::string::npos){//UTF-8文字の場合
				hasUTF = true;
    			std::bitset<8> bset((int(item)));
			    if(bset.test(7) && bset.test(6)){           //11000000 先頭byteか
			    if(_count != 0){                            //UTFByteからUTF先頭Byteの時
			        outcode.push_back(_decodeUTF());
			        _utfs.clear();
			        _count = 1;
			    }
			    }
			    _utfs.push_back(int(item));
			    _count++;
			}else{
			    if(_count != 0){                            //UTFByteから通常文字の時
			        outcode.push_back(_decodeUTF());
			        _utfs.clear();
			        _count = 0;
			    }
			    outcode.push_back(std::to_string(int(item)));
			}
			
		}
		if(!_utfs.empty()){									//最後がUTF文字の時
			outcode.push_back(_decodeUTF());
			_utfs.clear();
			_count = 0;
		}
		return outcode;
	}
}

std::string UTFCheck::_decodeUTF(){
            std::stringstream ss;
            std::bitset<16> bin(0);
			if (_utfs.size() == 1) {
				ss << std::hex << _utfs[0];
			}
			else if(_utfs.size() == 2){
				bin = _shiftbit(bin, int(_utfs[0]) & 0x1F, 5);//110x xxxx
				bin = _shiftbit(bin, int(_utfs[1]) & 0x3F, 6);//10xx xxxx
				ss << std::hex << (int)bin.to_ulong();
			}
			else if (_utfs.size() == 3) {
				bin = _shiftbit(bin, int(_utfs[0]) & 0x0F, 4);//1110 xxxx
				bin = _shiftbit(bin, int(_utfs[1]) & 0x3F, 6);//10xx xxxx
				bin = _shiftbit(bin, int(_utfs[2]) & 0x3F, 6);//10xx xxxx
				ss << std::hex << (int)bin.to_ulong();
			}
			else if (_utfs.size() == 4) {
				bin = _shiftbit(bin, int(_utfs[0]) & 0x07, 3);//1111 0xxx
				bin = _shiftbit(bin, int(_utfs[1]) & 0x3F, 6);//10xx xxxx
				bin = _shiftbit(bin, int(_utfs[2]) & 0x3F, 6);//10xx xxxx
				bin = _shiftbit(bin, int(_utfs[3]) & 0x3F, 6);//10xx xxxx
				ss << std::hex << (int)bin.to_ulong();
			}
			return("u\\"+ss.str());
    }

bool UTFCheck::isUTF() {
	return hasUTF;
}

std::bitset<16> UTFCheck::_shiftbit(std::bitset<16> pre, int val, int shift) {
	std::bitset<16> bset(0);
	bset = pre << shift | std::bitset<16>(val);
	return bset;
}