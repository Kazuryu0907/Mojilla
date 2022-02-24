#include <vector>
#include <sstream>
#include <bitset>

class UTFCheck {
public:
	UTFCheck() {};
	std::vector<std::string> get(std::string str);
	bool isUTF();
private:
	std::vector<std::string> outcode;
	std::vector<int> _utfs;
    int _count = 0;
	bool hasUTF = false;
    std::string _decodeUTF();
    std::bitset<16> _shiftbit(std::bitset<16> pre, int val, int shift);
};