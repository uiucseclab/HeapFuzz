#include "snapshot.hpp"
#include <fstream>

typedef struct page_link {
  unsigned long start;
  unsigned long end;
  bool readable;
  bool writeable;
  bool executable;
  unsigned long offset;
  std::string pathname;

} page_link;

//parses a single line from /rpoc/id/maps
page_link parse_link(std::string line) {
  // std::cerr << line << std::endl;
  page_link plink = {};
  auto div = line.find("-");
  auto offset = 2 * div + 1;
  // auto offset = line.find(" ");
  // std::cout << line.substr(0,div) << std::endl;
  plink.start = std::stoull(line.substr(0, div), nullptr, 16);
  plink.end = std::stoull(line.substr(div + 1, offset), nullptr, 16);

  // std::cout << line.substr(0,div) << std::endl;
  // std::cout << line.substr(div+1,div) << std::endl;

  // std::cout << line[offset+2] << std::endl;
  // std::cout << line[offset+2] << std::endl;

  plink.readable = (line[offset + 1] == 'r');
  plink.writeable = (line[offset + 2] == 'w');
  plink.executable = (line[offset + 3] == 'x');

  // std::cout << line[offset+2] << std::endl;

  plink.offset = std::stoull(line.substr(offset + 6, 8), nullptr, 16);
  auto pos = line.find("/");
  if (pos == std::string::npos) {
    plink.pathname = "";
  } else {
    plink.pathname = line.substr(pos);
  }
  /*
          std::cout << plink.start << std::endl;
          std::cout << plink.end << std::endl;
          std::cout << plink.readable << std::endl;
          std::cout << plink.writeable << std::endl;
          std::cout << plink.executable << std::endl;
          std::cout << plink.offset << std::endl;
          std::cout << plink.pathname << std::endl;
  */
  return plink;
}

//return vector of page_links denoting the pages of the child process
std::vector<page_link> create_page_map(pid_t child) {
  //I literally parse the text /proc/id/maps to get page details
  //I know this is janky, forgive me
  //its actually pretty consistent, just not portable
  auto filename = "/proc/" + std::to_string(child) + "/maps";
  std::ifstream link_file(filename);
  std::string line;
  std::vector<page_link> pmap;

  while (std::getline(link_file, line)) {
    pmap.push_back(parse_link(line));
  }
  return pmap;
}

//save a single page for the snapshot
//TODO: finish this
void save_page(page_link page, FILE* snapshot_file) {
  if (page.readable && page.writeable) {
    unsigned long page_size = page.end - page.start;
    // unsigned long start = page.start;

    char* page_buf = (char*)page.start;
    std::cout << page.readable << page.writeable << page.executable
              << std::endl;
    // segfault
    // std::cout << page_buf[0]<< std::endl;

    fwrite(page_buf, sizeof(char), page_size, snapshot_file);
    // unsigned long offset = page.offset;
  } else {
    std::cout << page.readable << page.writeable << page.executable
              << std::endl;
  }
}

void generate_snapshot(pid_t child) {
  std::cout << "placeholder" << std::endl;
  // char buf[] = {'l','o','l'};
  auto pmap = create_page_map(child);
  // memory map writeable snapshot file
  // write each writeable page while maintaining offsets in struct
  auto* snapshot_file = fopen("test.snap", "wb");

  // fwrite(buf,sizeof(char),sizeof(buf),snaphsot_file);
  save_page(pmap[2], snapshot_file);
  fclose(snapshot_file);
  // snaphsot_file = mmap (0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
}
