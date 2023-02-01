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

#if defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_IO)

#include "asio/multiple_buffer_sequence.hpp"
#include "asio/detail/socket_types.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// Base helper class to translate buffers into the native multiple buffer
// representation.
class base_multiple_buffer_sequence_adapter
{
protected:
  typedef ASIO_MULTIPLE_BUFFER_SEQUENCE_STRUCT native_multiple_buffer_type;

  template <typename MultipleBufferSequence>
  void do_prepare_op(MultipleBufferSequence& source,
      native_multiple_buffer_type& destination)
  {
    typename MultipleBufferSequence::buffer_sequence_adapter_type&
        buffer_sequence_adapter = source.buffer_sequence_adapter();
    const typename MultipleBufferSequence::endpoint_type& endpoint = 
        source.endpoint();
    auto& hdr = ASIO_MULTIPLE_BUFFER_SEQUENCE_STRUCT_HDR_PTR(destination);
    auto& len = ASIO_MULTIPLE_BUFFER_SEQUENCE_STRUCT_LEN(destination);
    socket_ops::init_msghdr_msg_name(hdr.msg_name, endpoint.data());
    hdr.msg_namelen = static_cast<int>(endpoint.size());
    hdr.msg_iov = buffer_sequence_adapter.buffers();
    hdr.msg_iovlen = buffer_sequence_adapter.count();
    hdr.msg_control = NULL;
    hdr.msg_controllen = 0;
    hdr.msg_flags = 0;
    len = 0;
  }

  template <typename MultipleBufferSequence>
  void do_complete_op(native_multiple_buffer_type& source,
      MultipleBufferSequence& destination, const asio::error_code& ec)
  {
    typename MultipleBufferSequence::endpoint_type& endpoint = 
        destination.endpoint();
    auto const& hdr = ASIO_MULTIPLE_BUFFER_SEQUENCE_STRUCT_HDR_PTR(source);
    auto const& len = ASIO_MULTIPLE_BUFFER_SEQUENCE_STRUCT_LEN(source);
    if (!ec)
    {
      endpoint.resize(hdr.msg_namelen);
    }
    destination.do_complete(socket_base::message_flags(hdr.msg_flags),
        static_cast<std::size_t>(len), ec);
  }
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

#if defined(ASIO_STANDALONE)
  typedef typename std::remove_const<typename std::remove_reference<
    MultipleBufferSequence>::type>::type raw_multiple_buffer_sequence_type;
#else // defined(ASIO_STANDALONE)
  typedef typename boost::remove_const<typename boost::remove_reference<
    MultipleBufferSequence>::type>::type raw_multiple_buffer_sequence_type;
#endif // defined(ASIO_STANDALONE)

  typedef typename raw_multiple_buffer_sequence_type::reference reference;
  
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
    do_prepare_at(offset());
  }

  native_multiple_buffer_type* native_buffers()
  {
    return native_multiple_buffer_type_container_.data();
  }

  std::size_t offset() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.offset();
  }

  std::size_t count() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.count();
  }

  std::size_t native_buffer_size() const ASIO_NOEXCEPT
  {
    return native_multiple_buffer_type_container_.size();
  }

  std::size_t size() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.size();
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

  std::size_t operations_executed() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.operations_executed();
  }

  std::size_t bytes_transferred() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_.bytes_transferred();
  }
  
  void do_prepare_at(std::size_t offset)
  {
    if (offset >= multiple_buffer_sequence_.size())
    {
      throw std::out_of_range("offset not less than operations count");
    }
    std::size_t count_op = multiple_buffer_sequence_.size() - offset;
    native_multiple_buffer_type_container_.resize(count_op);
    for (std::size_t i = 0; i < count_op; ++i)
    {
      reference asio_multiple_buffer_sequence =
        multiple_buffer_sequence_.at(i + offset);
      native_reference native_multiple_buffer_sequence =
        native_multiple_buffer_type_container_.at(i);
      this->do_prepare_op(asio_multiple_buffer_sequence, 
        native_multiple_buffer_sequence);
    }
    multiple_buffer_sequence_.set_operations_executed(0);
    multiple_buffer_sequence_.set_bytes_transferred(0);
  }

  void do_prepare()
  {
    do_prepare_at(offset());
  }

  void do_complete_at(std::size_t offset, std::size_t operations_executed,
      const asio::error_code& ec)
  {
    if (offset >= multiple_buffer_sequence_.size())
    {
      throw std::out_of_range("offset not less than operations count");
    }
    std::size_t count_op = multiple_buffer_sequence_.size() - offset;
    std::size_t bytes_transferred = 0;
    for (std::size_t i = 0; i < count_op; ++i)
    {
      reference asio_multiple_buffer_sequence =
        multiple_buffer_sequence_.at(i + offset);
      native_reference native_multiple_buffer_sequence =
        native_multiple_buffer_type_container_.at(i);
      this->do_complete_op(native_multiple_buffer_sequence, 
        asio_multiple_buffer_sequence, ec);
      bytes_transferred += asio_multiple_buffer_sequence.bytes_transferred();
    }
    multiple_buffer_sequence_.set_operations_executed(operations_executed);
    multiple_buffer_sequence_.set_bytes_transferred(bytes_transferred);
  }

  void do_complete(std::size_t operations_executed,
      const asio::error_code& ec)
  {
    do_complete_at(offset(), operations_executed, ec);
  }

  void do_complete_at(std::size_t offset, std::size_t operations_executed, 
      std::size_t bytes_transferred, const asio::error_code& ec)
  {
    do_complete_at(offset, operations_executed, ec);
    if (multiple_buffer_sequence_.bytes_transferred() != bytes_transferred)
    {
      throw std::logic_error("bytes_transferred mismatch");
    }
  }

  void do_complete(std::size_t operations_executed, 
      std::size_t bytes_transferred, const asio::error_code& ec)
  {
    do_complete_at(offset(), operations_executed, bytes_transferred, ec);
  }
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_IO)

#endif // ASIO_DETAIL_MULTIPLE_BUFFER_SEQUENCE_ADAPTER_HPP
