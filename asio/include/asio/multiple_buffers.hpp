#ifndef ASIO_MULTIPLE_BUFFERS_HPP
#define ASIO_MULTIPLE_BUFFERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstddef>
#include <vector>

#if defined(__linux__)
# define ASIO_MULTIPLE_BUFFERS_PER_SYSCALL 1
#endif // defined(__linux__)

#if !defined(ASIO_MULTIPLE_BUFFERS_PER_SYSCALL)
# define ASIO_MULTIPLE_BUFFERS_PER_SYSCALL 1
#endif // !defined(ASIO_MULTIPLE_BUFFERS_PER_SYSCALL)

namespace asio {

namespace detail {

template <typename BufferSequence, typename EndpointType>
struct multiple_buffers_item {

  BufferSequence buffer;
  EndpointType endpoint;
  std::size_t transferred;

  multiple_buffers_item() {
    transferred = 0;
  }

  multiple_buffers_item(const BufferSequence& buffer_)
    : buffer(buffer_), transferred(0) {
  }

  multiple_buffers_item(const BufferSequence& buffer_, const EndpointType& endpoint_)
    : buffer(buffer_), endpoint(endpoint_), transferred(0) {
  }

};

} // namespace detail

template <typename BufferSequence, typename EndpointType>
class multiple_buffers {
public:
  typedef detail::multiple_buffers_item<BufferSequence, EndpointType>
    item_type;

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

  bool full() const {
    return size() == ASIO_MULTIPLE_BUFFERS_PER_SYSCALL;
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

#endif // ASIO_MULTIPLE_BUFFERS_HPP
