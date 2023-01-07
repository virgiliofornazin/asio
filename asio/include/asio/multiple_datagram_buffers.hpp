//
// multiple_datagram_buffers.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Support for multiple datagram buffers code patches on Linux operating system
// Copyright (c) 2023 virgilio Alexandre Fornazin (virgiliofornazin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_MULTIPLE_DATAGRAM_BUFFERS_HPP
#define ASIO_MULTIPLE_DATAGRAM_BUFFERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/buffer_sequence_adapter.hpp"
#include <cstddef>
#include <vector>

#if defined(ASIO_HAS_MULTIPLE_DATAGRAM_BUFFER_IO)
# define ASIO_MULTIPLE_DATAGRAMS_PER_SYSCALL 1024
#endif // defined(ASIO_HAS_MULTIPLE_DATAGRAM_BUFFER_IO)

#if !defined(ASIO_MULTIPLE_DATAGRAMS_PER_SYSCALL)
# define ASIO_MULTIPLE_DATAGRAMS_PER_SYSCALL 1
#endif // !defined(ASIO_MULTIPLE_DATAGRAMS_PER_SYSCALL)

namespace asio {

template <typename BufferSequence, typename EndpointType>
struct single_datagram_buffer {
  BufferSequence buffer;
  EndpointType endpoint;
  socket_base::message_flags flags;
  std::size_t transferred;

  single_datagram_buffer() 
    : flags(0), transferred(0) {
  }

  explicit single_datagram_buffer(const BufferSequence& buffer_)
    : buffer(buffer_), flags(0), transferred(0) {
  }

  explicit single_datagram_buffer(const BufferSequence& buffer_,
    const EndpointType& endpoint_)
      : buffer(buffer_), endpoint(endpoint_), flags(0), transferred(0) {
  }

  explicit single_datagram_buffer(const BufferSequence& buffer_,
    const EndpointType& endpoint_, socket_base::message_flags const& flags_)
      : buffer(buffer_), endpoint(endpoint_), flags(flags_), transferred(0) {
  }

  bool all_empty() const {
    return detail::buffer_sequence_adapter<BufferSequence, 
        BufferSequence>(buffer).all_empty();
  }
};

template <typename BufferSequence, typename EndpointType>
class multiple_datagram_buffers {
public:
  typedef single_datagram_buffer<BufferSequence, EndpointType> item_type;

private:
  std::vector<item_type> m_buffers;

public:
  void clear() {
    m_buffers.clear();
  }

  std::size_t size() const {
    return m_buffers.size();
  }

  bool empty() const {
    return m_buffers.empty();
  }

  bool all_empty() const {
    if (empty()) {
      return true;
    }
    for (auto const& buffer: m_buffers) {
      if (!buffer.empty()) {
        return false;
      }
    }
    return true;
  }

  bool full() const {
    return size() == ASIO_MULTIPLE_DATAGRAMS_PER_SYSCALL;
  }

  item_type& add_buffer(const BufferSequence& buffer) {
    std::size_t index = size();
    m_buffers.push_back(item_type(buffer));
    return at(index);
  }

  item_type& add_buffer_endpoint(const BufferSequence& buffer, const EndpointType& endpoint) {
    std::size_t index = size();
    m_buffers.push_back(item_type(buffer, endpoint));
    return at(index);
  }

  item_type& at(std::size_t index) {
    return m_buffers.at(index);
  }

  const item_type& at(std::size_t index) const {
    return m_buffers.at(index);
  }
};

} // namespace asio

#endif // ASIO_MULTIPLE_DATAGRAM_BUFFERS_HPP
