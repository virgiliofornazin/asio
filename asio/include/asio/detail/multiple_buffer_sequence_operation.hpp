//
// multiple_buffer_sequence_operation.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO: update header with copyright of asio C++ library )
//
// Support for multiple datagram buffers code patches on Linux operating system
// Copyright (c) 2023 virgilio A. Fornazin (virgiliofornazin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_MULTIPLE_BUFFER_SEQUENCE_OPERATION_HPP
#define ASIO_MULTIPLE_BUFFER_SEQUENCE_OPERATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/buffer_sequence_adapter.hpp"
#include "asio/error_code.hpp"
#include "asio/socket_base.hpp"
#include <cstddef>
#include <vector>

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// a class that envelop standard asio const/mutable buffers sequences for 
// send/receive multiple datagrams within a single system call in supported 
// operating systems (Linux only at this time) 
/**
 * The @c buffer_sequence class envelop standard asio const/mutable 
 * buffers sequences for send/receive multiple datagrams within a single system 
 * call in supported operating systems (Linux only at this time) 
 */
template <typename BufferSequence, typename EndpointType>
class multiple_buffer_sequence_operation
{
public:
  typedef BufferSequence buffer_sequence_type;
  typedef typename buffer_sequence_type::native_buffer_type native_buffer_type;
  typedef buffer_sequence_adapter<buffer_sequence_type, buffer_sequence_type>
      buffer_sequence_adapter_type;
  typedef EndpointType endpoint_type;

public:
  multiple_buffer_sequence_operation() ASIO_NOEXCEPT
    : buffer_sequence_(), buffer_sequence_adapter_(buffer_sequence_), 
      endpoint_(), completed_(false), flags_(0), bytes_transferred_(0),
      error_code_()
  {
  }

  explicit multiple_buffer_sequence_operation(
      const buffer_sequence_type& _buffer_sequence)
      ASIO_NOEXCEPT
    : buffer_sequence_(_buffer_sequence), 
      buffer_sequence_adapter_(buffer_sequence_), endpoint_(), 
      completed_(false), flags_(0), bytes_transferred_(0),
      error_code_()
  {
  }

  explicit multiple_buffer_sequence_operation(
      const buffer_sequence_type& _buffer_sequence,
      const endpoint_type& _endpoint) ASIO_NOEXCEPT
    : buffer_sequence_(_buffer_sequence),  
      buffer_sequence_adapter_(buffer_sequence_), endpoint_(_endpoint), 
      completed_(false), flags_(0), bytes_transferred_(0),
      error_code_()
  {
  }

  multiple_buffer_sequence_operation(
      multiple_buffer_sequence_operation const& other)
    : buffer_sequence_(other.buffer_sequence_),  
      buffer_sequence_adapter_(buffer_sequence_), endpoint_(other.endpoint_), 
      completed_(other.completed_), flags_(other.flags_), 
      bytes_transferred_(other.bytes_transferred_),
      error_code_(other.error_code)
  {
  }

#if defined(ASIO_HAS_MOVE)
  multiple_buffer_sequence_operation(
      multiple_buffer_sequence_operation const&& other)
    : buffer_sequence_(std::move(other.buffer_sequence_)),
      buffer_sequence_adapter_(buffer_sequence_),
      endpoint_(std::move(other.endpoint_)),
      completed_(std::move(other.completed_)),
      flags_(std::move(other.flags_)),
      bytes_transferred_(std::move(other.bytes_transferred_)),
      error_code_(std::move(other.error_code))
  {
  }
#endif // defined(ASIO_HAS_MOVE)

  multiple_buffer_sequence_operation & operator = (
      multiple_buffer_sequence_operation const& other)
  {
    buffer_sequence_ = other.buffer_sequence_;
    buffer_sequence_adapter_ = buffer_sequence_adapter_type(buffer_sequence_);
    endpoint_ = other.endpoint_;
    completed_ = other.completed_;
    flags_ = other.flags_;
    bytes_transferred_ = other;bytes_transferred_;
    error_code_ = other.error_code_;

    return (*this);
  }

#if defined(ASIO_HAS_MOVE)
  multiple_buffer_sequence_operation & operator = (
      multiple_buffer_sequence_operation const&& other)
  {
    buffer_sequence_ = std::move(other.buffer_sequence_);
    buffer_sequence_adapter_ = buffer_sequence_adapter_type(buffer_sequence_);
    endpoint_ = std::move(other.endpoint_);
    completed_ = std::move(other.completed_);
    flags_ = std::move(other.flags_);
    bytes_transferred_ = std::move(other;bytes_transferred_);
    error_code_ = std::move(other.error_code_);

    return (*this);
  }
#endif // defined(ASIO_HAS_MOVE)

  native_buffer_type* buffers()
  {
    return buffer_sequence_adapter_.buffers();
  }

  std::size_t count() const
  {
    return buffer_sequence_adapter_.count();
  }

  std::size_t total_size() const
  {
    return buffer_sequence_adapter_.total_size();
  }

  registered_buffer_id registered_id() const
  {
    return buffer_sequence_adapter_.registered_id();
  }

  bool all_empty() const
  {
    return buffer_sequence_adapter_.all_empty();
  }
  
  void reset() ASIO_NOEXCEPT
  {
    buffer_sequence_ = buffer_sequence_type();
    buffer_sequence_adapter_ = buffer_sequence_adapter_type(buffer_sequence_);
    endpoint_ = endpoint_type();
    completed_ = false;
    flags_ = 0;
    bytes_transferred_ = 0;
    error_code_ = asio::error_code();
  }

  void reset(const buffer_sequence_type& _buffer_sequence) ASIO_NOEXCEPT
  {
    buffer_sequence_ = _buffer_sequence;
    buffer_sequence_adapter_ = buffer_sequence_adapter_type(buffer_sequence_);
    endpoint_ = endpoint_type();
    completed_ = false;
    flags_ = 0;
    bytes_transferred_ = 0;
    error_code_ = asio::error_code();
  }

  void reset(const buffer_sequence_type& _buffer_sequence,
      const endpoint_type& _endpoint) ASIO_NOEXCEPT
  {
    buffer_sequence_ = _buffer_sequence;
    buffer_sequence_adapter_ = buffer_sequence_adapter_type(buffer_sequence_);
    endpoint_ = _endpoint;
    completed_ = false;
    flags_ = 0;
    bytes_transferred_ = 0;
    error_code_ = asio::error_code();
  }

  bool empty() const
  {
    return all_empty();
  }

  buffer_sequence_type& buffer_sequence() ASIO_NOEXCEPT
  {
    return buffer_sequence_;
  }
  
  const buffer_sequence_type& buffer_sequence() const
      ASIO_NOEXCEPT
  {
    return buffer_sequence_;
  }

  buffer_sequence_adapter_type& buffer_sequence_adapter() ASIO_NOEXCEPT
  {
    return buffer_sequence_adapter_;
  }
  
  const buffer_sequence_adapter_type& buffer_sequence_adapter() const
      ASIO_NOEXCEPT
  {
    return buffer_sequence_adapter_;
  }

  void set_buffer_sequence(const buffer_sequence_type& _buffer_sequence)
      ASIO_NOEXCEPT
  {
    buffer_sequence_ = _buffer_sequence;
    buffer_sequence_adapter_ = buffer_sequence_adapter_type(buffer_sequence_);
  }
  
  endpoint_type& endpoint() ASIO_NOEXCEPT
  {
    return endpoint_;
  }
  
  const endpoint_type& endpoint() const ASIO_NOEXCEPT
  {
    return endpoint_;
  }
  
  void set_endpoint(const endpoint_type& _endpoint) const ASIO_NOEXCEPT
  {
    endpoint_ = _endpoint;
  }

  const bool completed() ASIO_NOEXCEPT
  {
    return completed_;
  }
  
  const socket_base::message_flags& flags() const ASIO_NOEXCEPT
  {
    return flags_;
  }
  
  std::size_t bytes_transferred() const ASIO_NOEXCEPT
  {
    return bytes_transferred_;
  }

  const asio::error_code& error_code() const ASIO_NOEXCEPT
  {
    return error_code_;
  }

  void complete_operation(socket_base::message_flags _flags,
      std::size_t _bytes_transferred, const asio::error_code& _error_code)
  {
    completed_ = true;
    flags_ = _flags;
    bytes_transferred_ = _bytes_transferred;
    error_code_ = _error_code;
  }

  void complete_operation(std::size_t _bytes_transferred,
      const asio::error_code& _error_code)
  {
    completed_ = true;
    bytes_transferred_ = _bytes_transferred;
    error_code_ = _error_code;
  }

private:
  buffer_sequence_type buffer_sequence_;
  buffer_sequence_adapter_type buffer_sequence_adapter_;
  endpoint_type endpoint_;
  bool completed_;
  socket_base::message_flags flags_;
  std::size_t bytes_transferred_;
  asio::error_code error_code_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_MULTIPLE_BUFFER_SEQUENCE_OPERATION_HPP
