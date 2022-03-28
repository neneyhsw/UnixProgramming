#include <bits/stdc++.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>  // for opendir

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <linux/limits.h>  // for PATH_MAX
#include <libgen.h>
#include <fcntl.h>
#include <stdio.h>
#include <pwd.h>
#include <iomanip>

using namespace std;

struct ProcInfo {
  string cmd;
  string pid;
  string user;
  string fd;
  string type;
  string node;
  string name;
};

void PrintHeader();
void PrintContent(struct ProcInfo &);
void ParamError();
bool IsNumber(const string &);
bool GetUser(string, string &);
bool GetArg(bool *, bool *, bool *, int, char **, string *, string *, string *);
bool GetCmd(string, string &, string, string, string);
int GetLinkProc(string, struct ProcInfo &, string, string, string, string);
int GetRootProc(string, struct ProcInfo &, string, string, string);
int GetExeProc(string, struct ProcInfo &, string, string, string);
int GetMemProc(string, struct ProcInfo &, string, string, string);
int GetFdProc(string, struct ProcInfo &, string, string, string);
bool GetName(string, struct ProcInfo &, bool);
void GetType(struct ProcInfo &, struct stat &);

void PrintHeader() {
  cout << "COMMAND\tPID\tUSER\tFD\tTYPE\tNODE\tNAME" << endl;
  return;
}

void PrintContent(struct ProcInfo &proc) {
  cout << proc.cmd << "\t" << proc.pid << "\t" << \
  proc.user << "\t" << proc.fd << "\t" << \
  proc.type << "\t" << proc.node << "\t" << \
  proc.name << endl;
  return;
}

void ParamError() {
  //cerr << "Please input correct format for hw1." << endl;
  cerr << "Usage: ./hw1 [OPTION]... [ARGUMENT]..." << endl;
  cerr << "-c REGEX: a regular expression (REGEX) filter for filtering command line." << endl;
  cerr << "-t TYPE: a TYPE filter. Valid TYPE includes REG, CHR, DIR, FIFO, SOCK, and unknown." << endl;
  cerr << "-f REGEX: a regular expression (REGEX) filter for filtering filenames." << endl;
  return;
}

// https://www.delftstack.com/zh-tw/howto/cpp/how-to-determine-if-a-string-is-number-cpp/
bool IsNumber(const string& str) {
  for (char const &c : str) {
    if (std::isdigit(c) == 0) return false;
  }
  return true;
}

bool GetUser(string dir, string &user){
  struct passwd *pws;
  struct stat statbuf;

  if (stat(dir.c_str(), &statbuf) < 0) {
    cerr << "stat error: " << errno << endl;
    return false;
  }
  if ((pws = getpwuid(statbuf.st_uid)) == NULL) {
    cerr << "getpwuid error: " << errno << endl;
    return false;
  }
  user = (string)pws->pw_name;
  return true;
}

// check argv status
bool GetArg(bool *c, bool *t, bool *f, int num, char *param[], string *strc, string *strt, string *strf) {
  string sc = "-c", st = "-t", sf = "-f";
  string t1 = "REG", t2 = "CHR", t3 = "DIR", t4 = "FIFO", t5 = "SOCK", t6 = "unknown";
  for (int i = 1; i < num; i++) {
    if (param[i] == sc) {
      if (i < (num - 1)) {
        if (param[i+1] == st || param[i+1] == sf)
          return false;
        *c = true;
        *strc = param[i+1];
      }
      else
        return false;
    }
    if (param[i] == st) {
      if (i < (num - 1)) {
        if (param[i+1] != t1 && param[i+1] != t2 && param[i+1] != t3 &&
          param[i+1] != t4 && param[i+1] != t5 && param[i+1] != t6) {
          cerr << "Invalid TYPE option." << endl;
          return false;
        }
        *t = true;
        *strt = param[i+1];
      }
      else {
        cerr << "Invalid TYPE option." << endl;
        return false;
      }
    }
    if (param[i] == sf) {
      if (i < (num - 1)) {
        if (param[i+1] == sc || param[i+1] == st)
          return false;
        *f = true;
        *strf = param[i+1];
      }
      else
        return false;
    }
  }

  // no param
  if (num == 1)
    return true;
  // no -c or -t or -f
  else if (!*c && !*t && !*f)
    return false;
  return true;
}

bool GetCmd(string work_dir, string &cmd, string c, string t, string f) {
  ifstream input_file(work_dir);
  string line;

  if (!input_file.is_open()) {
    cerr << "Could not open the file - '" << work_dir << "'" << endl;
    return false;
  }

  while (getline(input_file, line)) {
    // 檢查-c參數
    if (c == "" || line.find(c) != std::string::npos) {
      cmd = line;
    }
  }
  return true;
}

int RunProc(string work_dir, struct ProcInfo &proc, string c, string t, string f, string fd_type) {
  int res_int = 0;
  bool res_bool = false;
  if (fd_type == "cwd")
    res_int = GetLinkProc(work_dir + "/cwd", proc, c, t, f, "cwd");
  else if (fd_type == "root")
    res_int = GetLinkProc(work_dir + "/root", proc, c, t, f, "rtd");
  else if (fd_type == "exe")
    res_int = GetLinkProc(work_dir + "/exe", proc, c, t, f, "txt");
  else if (fd_type == "mem")
    res_int = GetMemProc(work_dir + "/mem", proc, c, t, f);
  else if (fd_type == "fd")
    res_int = GetFdProc(work_dir + "/fd", proc, c, t, f);
  return res_int;
}

// https://www.twblogs.net/a/5c71243fbd9eee68440f5c2e
/****
struct stat {
    dev_t st_dev; //device 文件的設備編號
    ino_t st_ino; //inode 文件的i-node
    mode_t st_mode; //protection 文件的類型和存取的權限
    nlink_t st_nlink; //number of hard links 連到該文件的硬連接數目, 剛建立的文件值爲1.
    uid_t st_uid; //user ID of owner 文件所有者的用戶識別碼 
    gid_t st_gid; //group ID of owner 文件所有者的組識別碼 
    dev_t st_rdev; //device type 若此文件爲裝置設備文件, 則爲其設備編號 
    off_t st_size; //total size, in bytes 文件大小, 以字節計算 
    unsigned long st_blksize; //blocksize for filesystem I/O 文件系統的I/O 緩衝區大小. 
    u nsigned long st_blocks; //number of blocks allocated 佔用文件區塊的個數, 每一區塊大小爲512 個字節. 
    time_t st_atime; //time of lastaccess 文件最近一次被存取或被執行的時間, 一般只有在用mknod、 utime、read、write 與tructate 時改變.
    time_t st_mtime; //time of last modification 文件最後一次被修改的時間, 一般只有在用mknod、 utime 和write 時間會改變
    time_t st_ctime; //time of last change i-node 最近一次被更改的時間, 此參數會在文件所有者、組、 權限被更改時更新 
};
****/
// 0 for general file, need to include TYPE and NODE
// 1 for NOFD
// 2 for error
int GetFdProc(string work_dir, struct ProcInfo &proc, string c, string t, string f) {
  DIR *dp;
  struct dirent *dirp;
  char buffer[PATH_MAX];
  struct stat stat_buffer;
  ssize_t ssize_num;

  if ((dp = opendir(work_dir.c_str())) == NULL) {
    // 表示此fd permission denied
    if (errno == EACCES) {
      proc.fd = "NOFD";
      proc.type = "";
      proc.node = "";
      proc.name = work_dir + " (Permission denied)";

      if ((proc.type.find(t) != std::string::npos || t == "") && \
        (proc.name.find(f) != std::string::npos || f == ""))
        PrintContent(proc);
      return 0;
    }
    //cerr << "opendir error: " << errno << endl;
    //return false;
  }
  while ((dirp = readdir(dp)) != NULL) {
    if (IsNumber(dirp->d_name)) {
      string current_dir = work_dir + "/" + dirp->d_name;

      // get proc name from readkink
      if ((ssize_num = readlink(current_dir.c_str(), buffer, PATH_MAX)) < 0) {
        cerr << "readlink error: " << errno << endl;
        return 2;
      }
      if (stat(current_dir.c_str(), &stat_buffer) < 0) {
        cerr << "stat error: " << errno << endl;
        return 2;
      }
      GetType(proc, stat_buffer);

      // get node number
      auto ino = stat_buffer.st_ino;
      proc.node = to_string(ino);
      //string right_end = "]";
      if (proc.type == "FIFO")
        proc.name = "pipe:[" + proc.node + (string)"]";
      else if (proc.type == "SOCK")
        proc.name = "socket:[" + proc.node + (string)"]";
      else
        proc.name = string(dirname(buffer));  // get proc name from buffer

      // http://naeilproj.blogspot.com/2015/08/linux-c-c.html
      // check access permission
      proc.fd = dirp->d_name;
      if (access(current_dir.c_str(), R_OK | W_OK) == 0)
        proc.fd += "u";
      else if (access(current_dir.c_str(), R_OK) == 0)
        proc.fd += "r";
      else if (access(current_dir.c_str(), W_OK) == 0)
        proc.fd += "w";

      if ((proc.type.find(t) != std::string::npos || t == "") && \
        (proc.name.find(f) != std::string::npos || f == ""))
        PrintContent(proc);
    }
  }
  closedir(dp);
  return 0;
}

bool GetName(string work_dir, struct ProcInfo &proc, bool is_open_fd) {
  if (!is_open_fd) {
    proc.name = work_dir + "/fd (Permission denied)";
  }
  return true;
}

void GetType(struct ProcInfo &proc, struct stat &stat_buffer) {
  // https://blog.csdn.net/astrotycoon/article/details/8679676
  if (stat_buffer.st_mode & S_IFMT == S_IFDIR)
    proc.type = "DIR";
  else if (stat_buffer.st_mode & S_IFMT == S_IFREG)
    proc.type = "REG";
  else if (stat_buffer.st_mode & S_IFMT == S_IFCHR)
    proc.type = "CHR";
  else if (stat_buffer.st_mode & S_IFMT == S_IFIFO)
    proc.type = "FIFO";
  else if (stat_buffer.st_mode & S_IFMT == S_IFSOCK)
    proc.type = "SOCK";
  else
    proc.type = "unknown";
  return;
}

int GetLinkProc(string work_dir, struct ProcInfo &proc, string c, string t, string f, string link_name) {
  char buffer[PATH_MAX];
  struct stat stat_buffer;
  ssize_t ssize_num;

  // get filename from readlink
  if ((ssize_num = readlink(work_dir.c_str(), buffer, PATH_MAX)) < 0) {
    // 沒有存取權限
    if (errno == EACCES) {
      proc.type = "unknown";
      proc.fd = link_name;
      proc.name = work_dir + " (Permission denied)";
      if ((proc.type.find(t) != std::string::npos || t == "") && \
        (proc.name.find(f) != std::string::npos || f == ""))
        PrintContent(proc);
      return 0;
    }
    // no such file or directory
    //else if (errno == ENOENT)
    //  return 2;
    //return 2;
  }
  else {
    if (stat(work_dir.c_str(), &stat_buffer) < 0) 
      cerr << "stat error: " << errno << endl;

    GetType(proc, stat_buffer);
    auto ino = stat_buffer.st_ino;
    proc.node = to_string(ino);
    proc.fd = link_name;

    if (proc.type == "FIFO")
      proc.name = "pipe:[" + proc.node + (string)"]";
    else if (proc.type == "SOCK")
      proc.name = "socket:[" + proc.node + (string)"]";
    else
      proc.name = string(dirname(buffer));  // get proc name from buffer

    //proc.type = "DIR";
    //proc.name = string(dirname(buffer));

    if ((proc.type.find(t) != std::string::npos || t == "") && \
      (proc.name.find(f) != std::string::npos || f == ""))
      PrintContent(proc);
  }

  return 0;
}

int GetMemProc(string work_dir, struct ProcInfo &proc, string c, string t, string f) {
  // 檢查檔案是否能開啟
  ifstream input_file(work_dir);
  string line;
  if (!input_file.is_open()) {
    cerr << "Could not open the file - '" << work_dir << "'" << endl;
    return 2;
  }

  char buffer[PATH_MAX];
  struct stat stat_buffer;
  ssize_t ssize_num;

  

}

int main(int argc, char *argv[]) {
  bool input_flag = true;
  bool flagc = false, flagt = false, flagf = false;
  string strc = "", strt = "", strf = "";
  input_flag = GetArg(&flagc, &flagt, &flagf, argc, argv, &strc, &strt, &strf);  // check which parameter is input

  if (!input_flag) {
    ParamError();
    return EXIT_FAILURE;
  }
  PrintHeader();

  string proc_dir = "/proc/";
  // https://www.delftstack.com/zh-tw/howto/c/opendir-in-c/
  DIR *dp;
  struct dirent *dirp;
  if ((dp = opendir(proc_dir.c_str())) == NULL) {
    cerr << "opendir error: " << errno << endl;
    return EXIT_FAILURE;
  } 
  bool is_cmd = false, is_user = false, is_name = false;
  int fd_num = 0, type_num = 0, run_res = 0;
  while ((dirp = readdir(dp)) != NULL) {
    if (IsNumber(dirp->d_name)) {
      struct ProcInfo proc;
      proc.pid = dirp->d_name;
      string work_dir = proc_dir + proc.pid;
      if ((is_cmd = GetCmd(work_dir + "/comm", proc.cmd, strc, strt, strf)) == false) {
        cerr << "get cmd failed: " << errno << endl;
        continue;
      }
      // handle -c param
      if (proc.cmd == "")
        continue;

      if ((is_user = GetUser(work_dir, proc.user)) == false) {
        cerr << "get user failed: " << errno << endl;
        continue;
      }


      //if ((fd_num = GetFdProc(work_dir + "/fd", proc, strc, strt, strf)) == 2) {
      //  cerr << "get fd failed: " << errno << endl;
      //  continue;
      //}

      if ((run_res = RunProc(work_dir, proc, strc, strt, strf, "cwd")) != 0) {
        cerr << "run cwd proc error: " << errno << endl;
        continue;
      }
      if ((run_res = RunProc(work_dir, proc, strc, strt, strf, "root")) != 0) {
        cerr << "run root proc error: " << errno << endl;
        continue;
      }

      if ((run_res = RunProc(work_dir, proc, strc, strt, strf, "fd")) != 0) {
        cerr << "run fd proc error: " << errno << endl;
        continue;
      }


      /*
      // find name according to is_fd number
      if ((is_name = GetName(work_dir, proc, fd_num)) == false) {
        cerr << "get name error: " << errno << endl;
        continue;
      }
      if (fd_num == 1) {
        PrintContent(proc);
        continue;
      }
      */

      /*
      if ((type_num = GetType(work_dir + "/fd", proc, strc, strt, strf)) == 2) {
        cerr << "get type error: " << errno << endl;
        continue;
      }
      */

      //PrintContent(proc);

    }
  }
  closedir(dp);

  return EXIT_SUCCESS;
}

// read file in c language
  /*
  FILE *f;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  f = fopen(work_dir.c_str(), "r");
  if (f == NULL) {
    cout << work_dir << endl;
    cerr << "open file failed:" << errno << endl;
    return false;
  }
  else {
    while (read = getline(&line, &len, f) != EOF) {
      cmd = line;
    }
    fclose(f);
  }
  return true;
  */

