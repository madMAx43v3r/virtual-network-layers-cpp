/*
 * UdpClient.h
 *
 *  Created on: Sep 26, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_UDPCLIENT_H_
#define INCLUDE_VNL_UDPCLIENT_H_

#include <vnl/Packet.h>

#include <sys/types.h>
#include <sys/socket.h>


namespace vnl {

template<int N>
class UdpClient {
public:
	UdpClient(int sock, sockaddr dest_addr)
		:	sock(sock), dest_addr(dest_addr), seq_num(0)
	{
		assert(N+tail_len <= Page::size);
		data = Page::alloc();
	}
	
	~UdpClient() {
		data->free_all();
	}
	
	int send(vnl::Packet* pkt) {
		seq_num++;
		vnl::io::BasicByteBuffer<Page, N> buf;
		vnl::io::TypeOutput out(&buf);
		buf.wrap(data);
		pkt->serialize(out);
		out.flush();
		int num_bytes = buf.position();
		int num_pages = buf.position() / N;
		if(num_bytes > num_pages * N) {
			num_pages++;
		}
		if(num_pages > 32000) {
			return 0;
		}
		Page* chunk = data;
		int left = num_bytes;
		int sent = 0;
		for(int i = 0; i < num_pages; ++i) {
			int size = std::min(N, left);
			set_tail(chunk, size, num_pages, i);
			sent += send_chunk(chunk, size);
			chunk = chunk->next;
			left -= N;
		}
		return sent == num_bytes ? num_bytes : -1;
	}
	
protected:
	int send_chunk(Page* chunk, int size) {
		return sendto(sock, chunk->mem, size+tail_len, 0, &dest_addr, sizeof(dest_addr));
	}
	
private:
	void set_tail(Page* chunk, int size, int num_pages, int index) {
		chunk->type_at<uint32_t>(size) = vnl_htonl(seq_num);
		chunk->type_at<uint16_t>(size+4) = vnl_htonl(num_pages);
		chunk->type_at<uint16_t>(size+6) = vnl_htonl(index);
	}
	
private:
	int sock;
	sockaddr dest_addr;
	
	uint32_t seq_num;
	Page* data;
	
	static const int tail_len = 8;
	
};


} // vnl

#endif /* INCLUDE_VNL_UDPCLIENT_H_ */
