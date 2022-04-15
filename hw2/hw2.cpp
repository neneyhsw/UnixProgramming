#include <bits/stdc++.h>
#include <unistd.h>
#define BUFFER 10000
using namespace std;

void error_msg() {
  cerr << "usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n" << \
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
  char c = {0};
  char out_from_optarg[BUFFER];
  while((c = getopt(argc, argv, "o:p:")) != -1) {
    switch(c) {
      case 'o':
        cout << "in o" << endl;
        break;
      case 'p':
        cout << "in p" << endl;
        break;
      default:
        error_msg();
        break;
    }
  }

  // get -- param
  vector<string> cmd_v;
  for (int i = optind; i < argc; i++)
    cmd_v.push_back(argv[i]);

  // for testing
  for (int i = 0; i < cmd_v.size(); i++)
    cout << cmd_v[i] << endl;

  return EXIT_SUCCESS;
}

