#include <string>

class INotifyd {
public:
	INotifyd();
	bool init();
	bool watch();
private:
	std::string describeEvent(int mask );
	int fd;
}
