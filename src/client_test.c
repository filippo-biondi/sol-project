#include <utils.h>
#include <i_conn.h>

int main(int argc, char* argv[])
{
  struct timespec abstime;
  char stringa[] = "sono la stringa che verrà appesa";
  
  if(openFile("random_file/rf1.txt", O_CREATE) != 0)
  {
    perror("Open error");
  }
  
  clock_gettime(CLOCK_REALTIME, &abstime);
  abstime.tv_sec += MAX_WAIT_TIME;
  
  if(openConnection("socket", 100, abstime) != 0)
  {
    perror("Connection error");
  }
  
  if(openFile("random_file/rf1.txt", O_CREATE) != 0)
  {
    perror("Open error");
  }
  
  if(writeFile("random_file/rf1.txt", NULL) != 0)
  {
    perror("Write error");
  }
  
  if(appendToFile("random_file/rf1.txt", &stringa, 33, NULL) != 0)
  {
    perror("Append error");
  }
  
  if(closeFile("random_file/rf1.txt") != 0)
  {
    perror("Close error");
  }
}
