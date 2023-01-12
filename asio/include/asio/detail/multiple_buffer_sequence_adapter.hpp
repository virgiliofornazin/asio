//
// detail/multiple_buffer_sequence_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO-MBS: update header with copyright of asio C++ library )
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

#include "asio/multiple_buffer_sequence.hpp"
#include "asio/detail/socket_types.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// Base helper class to translate buffers into the native multiple buffer
// representation.
class base_multiple_buffer_sequence_adapter
{
public:
#if defined(__linux__)
  typedef struct mmsghdr native_multiple_buffer_type;

  template <typename MultipleBufferSequence>
  void do_prepare_op(MultipleBufferSequence& source,
      native_multiple_buffer_type& destination)
  {
    typename MultipleBufferSequence::buffer_sequence_adapter_type&
        buffer_sequence_adapter = source.buffer_sequence_adapter();
    const typename MultipleBufferSequence::endpoint_type& endpoint = 
        source.endpoint();
    socket_ops::init_msghdr_msg_name(destination.msg_hdr.msg_name,
        endpoint.data());
    destination.msg_hdr.msg_namelen = static_cast<int>(endpoint.size());
    destination.msg_hdr.msg_iov = buffer_sequence_adapter.buffers();
    destination.msg_hdr.msg_iovlen = buffer_sequence_adapter.count();
    destination.msg_hdr.msg_control = NULL;
    destination.msg_hdr.msg_controllen = 0;
    destination.msg_hdr.msg_flags = 0;
  }

  template <typename MultipleBufferSequence>
  void do_complete_op(native_multiple_buffer_type& source,
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
#endif // defined(__linux__)
};

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
  // TODO-MBS: specialize native_multiple_buffer_type in asio::detail::array 
  // for fixed size multiple buffer sequence object
  typedef std::vector<native_multiple_buffer_type>
      native_multiple_buffer_type_container_type;

  multiple_buffer_sequence_type& multiple_buffer_sequence_;

  native_multiple_buffer_type_container_type
      native_multiple_buffer_type_container_;

public:
  explicit multiple_buffer_sequence_adapter(
      multiple_buffer_sequence_type& _multiple_buffer_sequence)
    : multiple_buffer_sequence_(_multiple_buffer_sequence)
  {
    do_prepare();
  }

  native_multiple_buffer_type* native_buffers()
  {
    return native_multiple_buffer_type_container_.data();
  }

  std::size_t native_buffer_size() const ASIO_NOEXCEPT
  {
    return native_multiple_buffer_type_container_.size();
  }

  std::size_t count() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.count();
  }

  std::size_t total_size() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.total_size();
  }

  bool all_empty() const
  {
    return multiple_buffer_sequence_.all_empty();
  }

  bool full() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.full();
  }

  std::size_t completed_ops() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.completed_ops();
  }

  std::size_t bytes_transferred() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.bytes_transferred();
  }

  void do_prepare()
  {
    std::size_t const count_op = multiple_buffer_sequence_.size();
    native_multiple_buffer_type_container_.resize(count_op);
    for (std::size_t i = 0; i < count_op; ++i)
    {
      reference asio_multiple_buffer_sequence = multiple_buffer_sequence_.at(i);
      native_reference native_multiple_buffer_sequence =
          native_multiple_buffer_type_container_.at(i);
      this->do_prepare_op(asio_multiple_buffer_sequence, 
          native_multiple_buffer_sequence);
    }
    multiple_buffer_sequence_.set_completed_ops(0);
    multiple_buffer_sequence_.set_bytes_transferred(0);
  }

  void do_complete(std::size_t completed_ops,
      const asio::error_code& ec)
  {
    std::size_t bytes_transferred = 0;
    std::size_t const count_op = multiple_buffer_sequence_.size();
    for (std::size_t i = 0; i < count_op; ++i)
    {
      reference asio_multiple_buffer_sequence = multiple_buffer_sequence_.at(i);
      native_reference native_multiple_buffer_sequence =
          native_multiple_buffer_type_container_.at(i);
      this->do_complete_op(native_multiple_buffer_sequence, 
          asio_multiple_buffer_sequence, ec);
      bytes_transferred += asio_multiple_buffer_sequence.bytes_transferred();
    }
    multiple_buffer_sequence_.set_completed_ops(completed_ops);
    multiple_buffer_sequence_.set_bytes_transferred(bytes_transferred);
  }

  void do_complete(std::size_t completed_ops, 
      std::size_t bytes_transferred, const asio::error_code& ec)
  {
    std::size_t const count_op = multiple_buffer_sequence_.size();
    for (std::size_t i = 0; i < count_op; ++i)
    {
      reference asio_multiple_buffer_sequence = multiple_buffer_sequence_.at(i);
      native_reference native_multiple_buffer_sequence =
          native_multiple_buffer_type_container_.at(i);
      this->do_complete_op(native_multiple_buffer_sequence, 
          asio_multiple_buffer_sequence, ec);
    }
    multiple_buffer_sequence_.set_completed_ops(completed_ops);
    multiple_buffer_sequence_.set_bytes_transferred(bytes_transferred);
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/buffer_sequence_adapter.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_MULTIPLE_BUFFER_SEQUENCE_ADAPTER_HPP
