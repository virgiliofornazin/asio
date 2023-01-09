//
// detail/multiple_buffer_sequence_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO: update header with copyright of asio C++ library )
//
// Support for multiple datagram buffers code patches on Linux operating system
// Copyright (c) 2023 virgilio A. Fornazin (virgiliofornazin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_MULTIPLE_BUFFER_SEQUENCE_ADAPTER_HPP
#define ASIO_DETAIL_MULTIPLE_BUFFER_SEQUENCE_ADAPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_IO)

#include "asio/multiple_buffer_sequence.hpp"

#if defined(__linux__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif // !defined(_GNU_SOURCE)
#include <sys/socket.h>
#endif // defined(__linux__)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// Base helper class to translate buffers into the native multiple buffer
// representation.
class base_multiple_buffer_sequence_adapter
{
public:
#if defined(__linux__)
#if defined(_GNU_SOURCE)
  typedef struct mmsghdr native_multiple_buffer_type;

  template <typename MultipleBufferSequence>
  void do_prepare(MultipleBufferSequence& source,
      native_multiple_buffer_type& destination)
  {
    typename MultipleBufferSequence::buffer_sequence_adapter_type&
        buffer_sequence_adapter = source.buffer_sequence_adapter();
    const typename MultipleBufferSequence::endpoint_type& endpoint = 
        source.endpoint();
    socket_ops::init_msghdr_msg_name(destination.msg_hdr.msg_name,
        endpoint.data());
    destination.msg_hdr.msg_namelen = static_cast<int>(endpoint.size);
    destination.msg_hdr.msg_iov = buffer_sequence_adapter.buffers();
    destination.msg_hdr.msg_iovlen = buffer_sequence_adapter.count();
    destination.msg_hdr.msg_control = NULL;
    destination.msg_hdr.msg_controllen = 0;
    destination.msg_hdr.msg_flags = 0;
  }

  template <typename MultipleBufferSequence>
  void do_complete(native_multiple_buffer_type& source,
      MultipleBufferSequence& destination, const asio::error_code& ec)
  {
    typename MultipleBufferSequence::endpoint_type& endpoint = 
        destination.endpoint();
    if (!ec)
    {
      endpoint.resize(source.msg_hdr.msg_namelen);
    }
    destination.do_complete(
        socket_base::message_flags(source.msg_hdr.msg_flags),
        static_cast<std::size_t>(source.msg_len), ec);
  }
#endif // defined(_GNU_SOURCE)
#endif // defined(__linux__)
};

/*
template <typename MultipleBufferSequence>
struct multiple_buffer_sequence_adapter_native_buffer_container
{
  const bool value = false;
};

template <typename BufferSequence, typename EndpointType, 
    std::size_t BufferSequenceCount>
struct multiple_buffer_sequence_adapter_native_buffer_container<
    fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType,
      BufferSequenceCount>>
{
  const bool value = true;

  typedef std::array<
      base_multiple_buffer_sequence_adapter::native_multiple_buffer_type,
      BufferSequenceCount> type;

  typedef typename type::size_type size_type;

  void resize(type&, size_type)
  {
  }
};

template <typename BufferSequence, typename EndpointType, 
    typename BufferSequenceContainerAllocatorType>
struct multiple_buffer_sequence_adapter_native_buffer_container<
    resizeable_multiple_buffer_sequence<BufferSequence, EndpointType,
      BufferSequenceContainerAllocatorType>>
{
  const bool value = true;

  typedef std::vector<
      base_multiple_buffer_sequence_adapter::native_multiple_buffer_type>
      type;

  typedef typename type::size_type size_type;

  void resize(type& container, size_type count)
  {
    container.resize(count);
  }
};
*/

// Helper class to translate buffers into the native multiple buffer
// representation.
template <typename MultipleBufferSequence>
class multiple_buffer_sequence_adapter
  : public base_multiple_buffer_sequence_adapter
{
public:
  typedef typename
    base_multiple_buffer_sequence_adapter::native_multiple_buffer_type
      native_multiple_buffer_type;
  typedef native_multiple_buffer_type& native_reference;

  typedef MultipleBufferSequence multiple_buffer_sequence_type;
  typedef typename multiple_buffer_sequence_type::reference reference;
  
private:
  typedef std::vector<native_multiple_buffer_type>
      native_multiple_buffer_type_container_type;

  multiple_buffer_sequence_type& multiple_buffer_sequence_;
  native_multiple_buffer_type_container_type
      native_multiple_buffer_type_container_;
  std::size_t completed_operations_;

private:
  void do_prepare()
  {
    std::size_t const operation_count = multiple_buffer_sequence_.size();
    native_multiple_buffer_type_container_.resize(operation_count);
    for (std::size_t i = 0; i < operation_count; ++i)
    {
      reference asio_multiple_buffer_sequence = multiple_buffer_sequence_.at(i);
      native_reference native_multiple_buffer_sequence =
          native_multiple_buffer_type_container_.at(i);
      this->do_prepare(asio_multiple_buffer_sequence, 
          native_multiple_buffer_sequence);
    }
  }

public:
  explicit multiple_buffer_sequence_adapter(
      multiple_buffer_sequence_type& _multiple_buffer_sequence)
    : multiple_buffer_sequence_(_multiple_buffer_sequence),
      completed_operations_(0)
  {
    do_prepare();
  }

  native_multiple_buffer_type* native_buffer()
  {
    return native_multiple_buffer_type_container_.data();
  }

  std::size_t size() const
  {
    return native_multiple_buffer_type_container_.size();
  }

  std::size_t completed_operations() const
  {
    return completed_operations_;
  }

  void do_complete(std::size_t _completed_operations,
      const asio::error_code& ec)
  {
    completed_operations_ = _completed_operations;
    
    /*
    std::size_t const operation_count = multiple_buffer_sequence_.size();
    native_multiple_buffer_type_container_.resize(operation_count);
    for (std::size_t i = 0; i < operation_count; ++i)
    {
      reference asio_multiple_buffer_sequence = multiple_buffer_sequence_.at(i);
      native_reference native_multiple_buffer_sequence =
          native_multiple_buffer_type_container_.at(i);
      this->do_prepare(asio_multiple_buffer_sequence, 
          native_multiple_buffer_sequence);
    }
    */
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/buffer_sequence_adapter.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_IO)

#endif // ASIO_DETAIL_MULTIPLE_BUFFER_SEQUENCE_ADAPTER_HPP
