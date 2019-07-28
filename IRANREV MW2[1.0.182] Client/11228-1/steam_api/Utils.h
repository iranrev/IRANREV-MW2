using namespace std;
using namespace System;  

//Some shits Added by Hosseinpourziyaie :)
//std::vector<std::string> split(const std::string& s, const std::string& delimiter, const bool& removeEmptyEntries = false);
string GetSpiltArrayValue(char *EntryText ,char *SplitterChar ,int Index);
const char* getMachineName();       
const char* getUserName();       
char* GetMacID();

void MarshalString ( String ^ s, string& os );  
void MarshalString ( String ^ s, wstring& os );  
