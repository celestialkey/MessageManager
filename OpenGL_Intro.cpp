#include <iostream>
#include "core\messageManager.h"
#include <thread>



void cbt1(geManagers::sMessage& msg) { std::cout << "1] Got Message with id " << msg.iUniqueMessageId << std::endl; }
void cbt2(geManagers::sMessage& msg) { std::cout << "2] Got Message with id " << msg.iUniqueMessageId << std::endl; }
void cbt3(geManagers::sMessage& msg) { std::cout << "3] Got Message with id " << msg.iUniqueMessageId << std::endl; }
void cbt4(geManagers::sMessage& msg) { std::cout << "4] Got Message with id " << msg.iUniqueMessageId << std::endl; }


void test() {
	geManagers::MessageManager *MsgPumpTest = new geManagers::MessageManager();

	MsgPumpTest->RegisterEvent("ExampleEvent1", 1);
	MsgPumpTest->RegisterEvent("ExampleEvent2", 2);
	MsgPumpTest->RegisterEvent("ExampleEvent3", 3);
	MsgPumpTest->RegisterEvent("ExampleEvent4", 4);

	std::thread t(&geManagers::MessageManager::Start, MsgPumpTest);

	MsgPumpTest->Subscribe(1, cbt1);
	MsgPumpTest->Subscribe(2, cbt2);
	MsgPumpTest->Subscribe(3, cbt3);
	MsgPumpTest->Subscribe(4, cbt4);
	
	int i = 0;
	geManagers::sMessage myMessage{ i };
	while (i<20) {
		Sleep(1);
		myMessage.iMessageId = i;
		int target = (rand() % 4) + 1;
		MsgPumpTest->QueueMessage(target, myMessage);
		i++;
	}

	while (MsgPumpTest->Pending()) { Sleep(1); };
	MsgPumpTest->Die();
	t.join();
}

int main(){
	test();
	return 0;
}
