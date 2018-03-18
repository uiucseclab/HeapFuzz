#include "snapshot.hpp"
#include <fstream>

page_link parse_link(std::string line){
	//std::cerr << line << std::endl;
	page_link plink = {};
	auto div = line.find("-");
	auto offset = 2*div + 1;
	//auto offset = line.find(" ");
	//std::cout << line.substr(0,div) << std::endl;
	plink.start = std::stoull(line.substr(0,div),nullptr,16);
	plink.end = std::stoull(line.substr(div+1,offset),nullptr,16);

	//std::cout << line.substr(0,div) << std::endl;
	//std::cout << line.substr(div+1,div) << std::endl;

	//std::cout << line[offset+2] << std::endl;
	//std::cout << line[offset+2] << std::endl;

	plink.readable = (line[offset+1] == 'r');
	plink.writeable = (line[offset+2] == 'w');
	plink.executable = (line[offset+3] == 'x');

	//std::cout << line[offset+2] << std::endl;

	plink.offset = std::stoull(line.substr(offset+6,8),nullptr,16);
	auto pos = line.find("/");
	if(pos == std::string::npos){
		plink.pathname = "";
	}
	else{
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

std::vector<page_link> create_page_map(pid_t child) {
	auto filename = "/proc/" + std::to_string(child) + "/maps";
	std::ifstream link_file(filename);
	std::string line; 
	std::vector<page_link> pmap;

	 while (std::getline(link_file, line)) {

	 	pmap.push_back(parse_link(line));	 	
  }
  return pmap;
}

//void save_snapshot(pid_t child, std::vector<page_link> pmap){}

void generate_snapshot(pid_t child){
  std::cout << "placeholder" << std::endl;
  auto pmap = create_page_map(child);

}

/*
auto fd = open64(temp_snapshot_path,
                   O_CREAT | O_CLOEXEC | O_RDWR | O_TRUNC, 0666);

ftruncate64(fd, static_cast<off64_t>(size));

auto file = reinterpret_cast<detail::Snapshot32File *>(
      mmap64(nullptr, size,
             PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));

//save_regs
//save_fp regs



*/