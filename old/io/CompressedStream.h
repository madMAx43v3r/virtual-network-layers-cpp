/*
 * Compressedstream->h
 *
 *  Created on: Mar 3, 2016
 *      Author: mad
 */

#ifndef INCLUDE_IO_COMPRESSEDSTREAM_H_
#define INCLUDE_IO_COMPRESSEDSTREAM_H_

#include <unordered_map>

namespace vnl { namespace io {

class CompressedStream : public Stream {
protected:
	
	static const int MAX_SIZE = 64;
	static const int MAX_NUM = 16384;
	
	static const int FLAG_SYM = 0 << 6;
	static const int FLAG_NEW = 1 << 6;
	static const int FLAG_RAW = 2 << 6;
	static const int FLAG_BYTE = 3 << 6;
	
	struct symbol {
		char id[2];
		char len = 0;
		char bytes[MAX_SIZE];
		bool used = false;
	};
	
	uint64_t bin_hashfunc(const char* src, int len) {
		switch(len) {
		case 1: return 0x7777777700ull | *src;
		case 2: return 0x777777770000ull | *((uint16_t*)src);
		case 4: return 0x7777777700000000ull | *((uint32_t*)src);
		case 8: return *((uint64_t*)src);
		}
		uint64_t h = 0;
		while(len > 8) {
			h ^= *((uint64_t*)src);
			src += 8;
			len -= 8;
		}
		for(int i = 0; i < len; ++i) {
			h ^= ((uint64_t)src[i]) << (i*8);
		}
		return h;
	}
	
public:
	CompressedStream(Stream* stream) : stream(stream) {
		in = new symbol[MAX_NUM];
		out = new symbol[MAX_NUM];
		for(int i = 0; i < MAX_NUM; ++i) {
			char id[2];
			id[0] = i >> 8;
			id[1] = i & 0xFF;
			in[i].id = id;
			out[i].id = id;
		}
	}
	
	~CompressedStream() {
		delete [] in;
		delete [] out;
	}
	
	virtual bool read(void* dst, int len) override {
		while(len) {
			if(left) {
				int n = std::min(left, len);
				if(current) {
					memcpy(dst, &current->bytes[current->len-left], n);
				} else {
					if(!stream->read(dst, n)) {
						return false;
					}
				}
				dst += n;
				len -= n;
				left -= n;
			} else {
				current = 0;
				char buf[2];
				if(!stream->read(buf, 2)) {
					return false;
				}
				uint16_t id = ((buf[0] & 0x3F) << 8) | buf[1];
				switch(buf[0] & 0xC0) {
				case FLAG_SYM:
					if(id < MAX_NUM) {
						symbol* sym = in + id;
						if(sym->len <= len) {
							memcpy(dst, sym->bytes, sym->len);
							if(sym->len == len) {
								return true;
							} else {
								dst += sym->len;
								len -= sym->len;
							}
						} else {
							current = sym;
							left = sym->len;
						}
					} else {
						return false;
					}
					break;
				case FLAG_NEW:
					if(id < MAX_NUM) {
						symbol* sym = in + id;
						if(!stream->read(&sym->len, 1)) {
							return false;
						}
						if(sym->len > MAX_SIZE || !stream->read(sym->bytes, sym->len)) {
							return false;
						}
						current = sym;
						left = sym->len;
					} else {
						return false;
					}
					break;
				case FLAG_RAW:
					left = id;
					break;
				case FLAG_BYTE:
					*dst = buf[1];
					dst++;
					len--;
					break;
				}
			}
		}
		return true;
	}
	
	virtual bool write(const void* src, int len) override {
		bytes_out += len;
		if(len <= MAX_SIZE) {
			if(len == 1) {
				count_bytes++;
				char buf[2];
				buf[0] = FLAG_BYTE;
				buf[1] = *((const char*)src);
				return stream->write(buf, 2);
			}
			uint64_t hash = bin_hashfunc((const char*)src, len);
			auto iter = hashmap.find(hash);
			if(iter != hashmap.end()) {
				symbol* sym = iter->second;
				if(len > 8) {
					if(len != sym->len || memcmp(src, sym->bytes, len)) {
						sym = 0;
					}
				}
				if(sym) {
					count_sym++;
					sym->used = true;
					return stream->write(sym, 2);
				}
			} else {
				symbol* sym = out + (pos++ % MAX_NUM);
				if(!sym->used) {
					count_new++;
					bytes_new += len;
					sym->len = len;
					memcpy(sym->bytes, src, len);
					hashmap[hash] = sym;
					sym->id[0] |= FLAG_NEW;
					bool res = stream->write(sym, 3+len);
					sym->id[0] &= 0x3F;
					return res;
				} else {
					sym->used = false;
				}
			}
		}
		count_raw++;
		bytes_raw += len;
		bool res = false;
		while(len) {
			int n = std::min(len, MAX_NUM-2);
			char buf[2];
			buf[0] = (n >> 8) | FLAG_RAW;
			buf[1] = n & 0xFF;
			res = stream->write(buf, 2);
			res &= stream->write(src, n);
			src += n;
			len -= n;
		}
		return res;
	}
	
	virtual bool flush() override {
		return stream->flush();
	}
	
	double get_comp_ratio() {
		return double(count_bytes*2 + count_sym*2 + count_new*3 + count_raw*2 + bytes_new + bytes_raw) / bytes_out;
	}
	
	uint64_t count_sym = 0;
	uint64_t count_new = 0;
	uint64_t count_raw = 0;
	uint64_t count_bytes = 0;
	uint64_t bytes_out = 0;
	uint64_t bytes_new = 0;
	uint64_t bytes_raw = 0;
	
private:
	Stream* stream;
	
	symbol* in;
	symbol* out;
	
	int left = 0;
	symbol* current = 0;
	
	unsigned int pos = 0;
	std::unordered_map<uint64_t, symbol*> hashmap;
	
	
};


}}

#endif /* INCLUDE_IO_COMPRESSEDSTREAM_H_ */
