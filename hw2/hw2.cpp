#include <bits/stdc++.h>
#include <fcntl.h>  /* For O_RDWR */
#include <unistd.h>
#include <sys/wait.h>
#define BUFFER 10000
using namespace std;

void ErrorMsg() {
  cout << "usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n" << \
    "-p: set the path to logger.so, default = ./logger.so\n" << \
    "-o: print output to file, print to \"stderr\" if no file specified\n" << \
    "--: separate the arguments for logger and for the command" << endl;
  return;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "no command given." << endl;
    return EXIT_FAILURE;
  }

  /* https://pubs.opengroup.org/onlinepubs/7908799/xsh/getopt.html */
  /* getopt can solve -- parameter */
  int c = 0;
  char *ofile = NULL;
  const char *pfile = "./logger.so";
  bool oflag = false, pflag = false;
  char out_from_optarg[BUFFER];
  while((c = getopt(argc, argv, "o:p:")) != -1) {
    switch(c) {
      case 'o':
        ofile = optarg;
        oflag = true;
        break;
      case 'p':
        pfile = optarg;
        pflag = true;
        break;
      default:
        ErrorMsg();
        break;
    }
  }

  /* get -- param */
  vector<string> cmd_v;
  for (int i = optind; i < argc; i++)
    cmd_v.push_back(argv[i]);

  int argcount = argc - optind;
  char **args = (char **) malloc((argcount + 1) * sizeof(char *));
  for (int i = 0; i < argcount; i++)
    args[i] = (char *) cmd_v[i].c_str();
  args[argcount] = NULL;

  pid_t pid = fork();
  int status = 0, fd = 0;
  if (pid < 0) {
    cerr << "fork error." << endl;
    return EXIT_FAILURE;
  }
  else if (pid == 0) {  // child
    if (oflag) {
      fd = open(ofile, O_CREAT|O_TRUNC|O_RDWR, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }

    // using logger.so to execute cmd
    setenv("LD_PRELOAD", pfile, 1);
    if (execvp(args[0], args) == -1) {
      fprintf(stderr, "Unknown command: [%s].\n", args[0]);
      exit(EXIT_FAILURE);
    }
    //cout << "pf: " << pfile << endl;
    //cout << "of: " << ofile << endl;
    //cout << "--: " << cmd_v[0] << endl;
  }
  else {  // parent
    while(waitpid(pid, &status, WNOHANG) != -1);
    //waitpid(pid, &status, 0);
  }

  return EXIT_SUCCESS;
}

