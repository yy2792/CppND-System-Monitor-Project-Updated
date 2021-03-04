#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  long totalMemo{0};
  long freeMemo{0};

  while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:"){
        totalMemo += stol(value);
      }
      if (key == "MemFree:"){
        freeMemo += stol(value);
      }
  }

  if (totalMemo != 0)
    return (static_cast<float>(totalMemo) - freeMemo) / totalMemo;
  else
    return 0;
 }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  string uptime, idletime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }
  return stol(uptime); 
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return ActiveJiffies() + IdleJiffies();
 }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    vector<string> parsedLine((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
    return stol(parsedLine[13]) + stol(parsedLine[14]) + stol(parsedLine[15]) + stol(parsedLine[16]);
  }
  return 0;
 }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  vector<string> cpu = CpuUtilization();
  long sum{0};
  sum += stol(cpu[LinuxParser::CPUStates::kUser_]);
  sum += stol(cpu[LinuxParser::CPUStates::kNice_]);
  sum += stol(cpu[LinuxParser::CPUStates::kSystem_]);
  sum += stol(cpu[LinuxParser::CPUStates::kIRQ_]);
  sum += stol(cpu[LinuxParser::CPUStates::kSoftIRQ_]);
  sum += stol(cpu[LinuxParser::CPUStates::kSteal_]);
  return sum;
 }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  vector<string> cpu = CpuUtilization();
  long sum{0};
  sum += stol(cpu[LinuxParser::CPUStates::kIdle_]);
  sum += stol(cpu[LinuxParser::CPUStates::kIOwait_]);
  return sum;
 }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  string key;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu"){
        vector<string> values((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
        return values;
      }
    }
  }
  return {};

 }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes"){
        return stoi(value);
      }
    }
  }

  return 0;
 }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running"){
        return stoi(value);
      }
    }
  }

  return 0;
 }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  string line;
  string command{};
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()){
    std::getline(stream, line);
    command = line;
  }
  return command;
 }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  string key, value;
  string result{};
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:"){
        result = to_string(stoi(value) / 1042);
      }
    } 
  }

  return result;

}

float LinuxParser::CpuUtilization(int pid) {
  string line;
  long utiltime;
  long elapsedtime;
  const long uptime = Jiffies();
  float result{0};
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    vector<string> parsedLine((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
    utiltime = stol(parsedLine[LinuxParser::ProcStates::cuTime_]) + stol(parsedLine[LinuxParser::ProcStates::csTime_]);
    elapsedtime = uptime - stol(parsedLine[LinuxParser::ProcStates::startTime_]);
    result = static_cast<float>(utiltime) / elapsedtime;
  }
  return result;
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  string key, uid;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> uid;
      if (key == "Uid:")
        return uid;
    }
  }
  return "Uid Error";
 }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  string line;
  string username = "User Error";
  std::ifstream stream(kPasswordPath);

  string uid = Uid(pid);
  if (uid != "Uid Error"){
    string searchUid = ":x:" + uid;
    if (stream.is_open()) {
      while (std::getline(stream, line)){
        auto pos = line.find(searchUid);
        username = line.substr(0, pos);
        return username;
      }
    }
  }
  return username;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  long systime = LinuxParser::UpTime();
  long startTime{0};
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    vector<string> parsedLine((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
    startTime = stol(parsedLine[21]);
  }
  return systime - startTime / sysconf(_SC_CLK_TCK);
 }
