#define BLKSIZE 512

struct iobuf {
      int fildes;
      int left;
      char *next;
      char buff[BLKSIZE];
};

