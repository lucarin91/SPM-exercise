/*
 * Naive file compressor using miniz, a single C source file Deflate/Inflate
 * compression
 * library with zlib-compatible API (Project page:
 * https://code.google.com/p/miniz/).
 *
 * To compile:
 *   g++ -std=c++11 -I$FF_ROOT -O3 simplecompressor.cpp -o simplecompressor
 */

#include "miniz.c"
#include "ff/pipeline.hpp"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>

using namespace std;
using namespace ff;

// map the file pointed by filepath in memory
static inline bool mapFile(const std::string& filepath,
                           unsigned char *  & ptr,
                           size_t           & size) {
  // open input file.
  int fd = open(filepath.c_str(), O_RDONLY);

  if (fd < 0) {
    printf("Failed opening file %s\n", filepath.c_str());
    return false;
  }
  struct stat s;

  if (fstat(fd, &s)) {
    printf("Failed to stat file %s\n", filepath.c_str());
    return false;
  }

  // checking for regular file type
  if (!S_ISREG(s.st_mode)) return false;

  // get the size of the file
  size = s.st_size;

  // map all the file in memory
  ptr = (unsigned char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (ptr == MAP_FAILED) {
    printf("Failed to map file %s in memory\n", filepath.c_str());
    return false;
  }
  close(fd);
  return true;
}

// unmap the file from memory
static inline void unmapFile(unsigned char *ptr, size_t size) {
  if (munmap(ptr, size) < 0) {
    printf("Failed to unmap file\n");
  }
}

// write size bytes starting from ptr into a file pointed by filename
static inline void writeFile(const std::string& filename,
                             unsigned char     *ptr,
                             size_t             size) {
  FILE *pOutfile = fopen(filename.c_str(), "wb");

  if (!pOutfile) {
    printf("Failed opening output file %s!\n", filename.c_str());
    return;
  }

  if (fwrite(ptr, 1, size, pOutfile) != size) {
    printf("Failed writing to output file %s\n", filename.c_str());
    return;
  }
  fclose(pOutfile);
}

struct myTask {
  string         filepath;
  unsigned char *ptr;
  unsigned long  size;
  myTask(string filepath, unsigned long size, unsigned char *ptr) : filepath(filepath), size(
      size),
    ptr(ptr) {}

  ~myTask() {
    delete[] ptr;
  }
};

struct ReadNode : ff_node_t<myTask>{
  vector<string>files;
  int           argc;

  ReadNode(int argc, char *argv[]) : argc(argc) {
    int  start     = 1;
    long num_files = argc - start;

    assert(num_files >= 1);

    for (long i = 0; i < num_files; ++i) {
      string filename(argv[start + i]);
      files.push_back(filename);
      cout << "parse argument, read filename " << filename << endl;
    }
  }

  myTask* svc(myTask *) {
    for (const string& file : this->files) {
      cout << "read file " << file << endl;
      unsigned char *ptr = nullptr;
      size_t size        = 0;

      if (!mapFile(file, ptr, size)){
        cerr << "failt to mapFile"<<endl;
        continue;
      }
      cout << "size after map " << size << endl;
      ff_send_out(new myTask(file, size, ptr));
    }
    return EOS;
  }
};

struct CompNode : ff_node_t<myTask>{
  myTask* svc(myTask *task) {
    // get an estimation of the maximum compression size
    unsigned long cmp_len = compressBound(task->size);

    // allocate memory to store compressed data in memory
    unsigned char *ptrOut = new unsigned char[cmp_len];

    if (compress(ptrOut, &cmp_len, task->ptr, task->size) != Z_OK) {
      cerr << "Failed to compress file in memory" <<endl;
      delete[] ptrOut;
      unmapFile(task->ptr, task->size);
      return EOS;
    }
    std::cout << "Compressed file from " << task->size <<
      " to " << cmp_len << endl;

    unmapFile(task->ptr, task->size);

    task->ptr  = ptrOut;
    task->size = cmp_len;
    return task;
  }
};

struct WriteNode : ff_node_t<myTask>{
  myTask* svc(myTask *task) {
    // get only the filename
    int n = task->filepath.find_last_of("/");
    string filename;

    if (n > 0) filename = task->filepath.substr(n + 1);
    else     filename = task->filepath;
    const string outfile = "./out/" + filename + ".zip";

    // write the compressed data into disk
    writeFile(outfile, task->ptr, task->size);

    cout << "write compressed file " << filename << " to the path " <<
      outfile << "\n";


    delete task;
    return GO_ON;
  }
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "use: " << argv[0] << " file [file]\n";
    return -1;
  }

  ReadNode  _1(argc, argv);
  CompNode  _2;
  WriteNode _3;
  ff_Pipe<> pipe(_1, _2, _3);

  if (pipe.run_and_wait_end() < 0) error("running pipe");

  return 0;
}
