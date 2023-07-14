#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

#define buf_size 512 //定义一个缓冲区的大小

int main(int argc, char *argv[]) {
  char buf[buf_size + 1] = {0};
  //uint 是一个数据类型，通常表示无符号整数
  uint occupy = 0;//记录缓冲区中已经占用的字节数
  //MAXARG 是一个常量，用于定义参数列表中最大的参数数量。kernel/param.h 中定义,32
  char *xargv[MAXARG] = {0};
  int stdin_end = 0;//标记标准输入是否已经读取完毕

  for (int i = 1; i < argc; i++) {
    xargv[i - 1] = argv[i];
  }

  while (!(stdin_end && occupy == 0)) {
    // try read from left-hand program
    if (!stdin_end) {
      int remain_size = buf_size - occupy;
      int read_bytes = read(0, buf + occupy, remain_size);
      if (read_bytes < 0) {
        fprintf(2, "xargs: read returns -1 error\n");
      }
      if (read_bytes == 0) {
        close(0);
        stdin_end = 1;
      }
      occupy += read_bytes;
    }
    // process lines read
    char *line_end = strchr(buf, '\n');
    while (line_end) {
      char xbuf[buf_size + 1] = {0};
      memcpy(xbuf, buf, line_end - buf);
      xargv[argc - 1] = xbuf;
      int ret = fork();
      if (ret == 0) {
        // i am child
        if (!stdin_end) {
          close(0);
        }
        if (exec(argv[1], xargv) < 0) {
          fprintf(2, "xargs: exec fails with -1\n");
          exit(1);
        }
      } else {
        // trim out line already processed
        memmove(buf, line_end + 1, occupy - (line_end - buf) - 1);//kao bei yi dong
        occupy -= line_end - buf + 1;
        memset(buf + occupy, 0, buf_size - occupy); //sheng yv she zhi 0
        // harvest zombie
        int pid;
        wait(&pid);

        line_end = strchr(buf, '\n');
      }
    }
  }
  exit(0);
}