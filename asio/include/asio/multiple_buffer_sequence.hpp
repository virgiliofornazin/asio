//
// multiple_buffer_sequence.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO-MBS: update header with copyright of asio C++ library )
//
// Support for multiple datagram buffers code patches on Linux operating system
// Copyright (c) 2023 virgilio A. Fornazin (virgiliofornazin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP
#define ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/array.hpp"
#include "asio/detail/multiple_buffer_sequence_op.hpp"
#include "asio/socket_base.hpp"
#include <stdexcept>
#include <memory>
#include <iterator>
#include <array>
#include <vector>
#if defined(ASIO_STANDALONE)
#include <type_traits>
#else // defined(ASIO_STANDALONE)
#include <boost/type_traits.hpp>
#endif // defined(ASIO_STANDALONE)

#include "asio/detail/push_options.hpp"

namespace asio {

ASIO_CONSTEXPR static const std::size_t
  multiple_buffer_sequence_maximum_operations_per_io = 1
#if defined(ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO)
  * ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO
#endif // defined(ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO)
;

template <typename BufferSequence, typename EndpointType>
class base_multiple_buffer_sequence
{
public:
  typedef BufferSequence buffer_sequence_type;
  typedef EndpointType endpoint_type;

  typedef detail::multiple_buffer_sequence_op<buffer_sequence_type, 
    endpoint_type> value_type;

protected:
  std::size_t offset_ = 0;
  std::size_t operations_executed_ = 0;
  std::size_t bytes_transferred_ = 0;

protected: 
  explicit base_multiple_buffer_sequence(std::size_t offset)
    : offset_(offset)
  {
  }

  void throw_empty() const
  {
    throw std::out_of_range(
      "no operations were assigned in multiple buffer sequence");
  }
  
public:
  base_multiple_buffer_sequence() = default;

  base_multiple_buffer_sequence(
    base_multiple_buffer_sequence const&) = default;
  
  base_multiple_buffer_sequence& operator = 
    (base_multiple_buffer_sequence const&) = default;

#if defined(ASIO_HAS_MOVE)
  base_multiple_buffer_sequence(
    base_multiple_buffer_sequence&&) = default;
  
  base_multiple_buffer_sequence& operator = 
    (base_multiple_buffer_sequence&&) = default;
#endif // defined(ASIO_HAS_MOVE)
  
  void throw_if_empty() const
  {
    if (empty())
    {
      throw_empty();
    }
  }

  virtual std::size_t size() const ASIO_NOEXCEPT = 0;

  virtual bool empty() const ASIO_NOEXCEPT
  {
    return size() <= 0;
  }

  virtual value_type& at(std::size_t index) = 0;
  
  virtual const value_type& at(std::size_t index) const = 0;

  value_type& operator[](std::size_t index)
  {
    return at(index);
  }

  const value_type& operator[](std::size_t index) const
  {
    return at(index);
  }

  std::size_t total_size() const ASIO_NOEXCEPT
  {
    if (empty())
    {
      return 0;
    }

    std::size_t result = 0;
    std::size_t const op_count = size();

    for (std::size_t index = offset_; index < op_count; ++index)
    {
      const value_type& buffer_sequence = at(index);

      result += buffer_sequence.total_size();
    }
    
    return result;
  }

  bool all_empty() const
  {
    if (empty())
    {
      return true;
    }

    std::size_t const op_count = size();

    for (std::size_t index = offset_; index < op_count; ++index)
    {
      const value_type& buffer_sequence = at(index);

      if (!buffer_sequence.all_empty())
      {
        return false;
      }
    }
    
    return true;
  }

  void reset()
  {
    std::size_t const op_count = size();

    for (std::size_t index = 0; index < op_count; ++index)
    {
      value_type& buffer_sequence = at(index);

      buffer_sequence.reset();
    }
  }

  std::size_t count() const ASIO_NOEXCEPT
  {
    return size();
  }

  std::size_t offset() const ASIO_NOEXCEPT
  {
    return offset_;
  }

  std::size_t operations_executed() const ASIO_NOEXCEPT
  {
    return operations_executed_;
  }

  void set_operations_executed(std::size_t _operations_executed) ASIO_NOEXCEPT
  {
    operations_executed_ = _operations_executed;
  }

  std::size_t add_operations_executed(std::size_t _operations_executed = 1) ASIO_NOEXCEPT
  {
    std::size_t prev = operations_executed_;

    operations_executed_ += _operations_executed;

    return prev;
  }

  std::size_t bytes_transferred() const ASIO_NOEXCEPT
  {
    return bytes_transferred_;
  }

  void set_bytes_transferred(std::size_t _bytes_transferred) ASIO_NOEXCEPT
  {
    bytes_transferred_ = _bytes_transferred;
  }

  std::size_t add_bytes_transferred(std::size_t _bytes_transferred) ASIO_NOEXCEPT
  {
    std::size_t prev = bytes_transferred_;

    bytes_transferred_ += _bytes_transferred;

    return prev;
  }
};

template <typename BufferSequence, typename EndpointType,
  typename ContainerType>
class base_multiple_buffer_sequence_template
  : public base_multiple_buffer_sequence<BufferSequence, EndpointType>
{
public:
  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType>
    base_type;

  typedef typename base_type::buffer_sequence_type buffer_sequence_type;
  typedef typename base_type::endpoint_type endpoint_type;
  typedef typename base_type::value_type value_type;

  typedef ContainerType container_type;

#if defined(ASIO_STANDALONE)
  typedef typename std::remove_const<typename std::remove_reference<
    ContainerType>::type>::type raw_container_type;
#else // defined(ASIO_STANDALONE)
  typedef typename boost::remove_const<typename boost::remove_reference<
    ContainerType>::type>::type raw_container_type;
#endif // defined(ASIO_STANDALONE)

protected:
#if defined(ASIO_HAS_STATIC_ASSERT)
  typedef typename raw_container_type::value_type container_value_type; 
  
  typedef typename container_value_type::buffer_sequence_type
    container_buffer_sequence_type;
  
  typedef typename container_value_type::endpoint_type
    container_endpoint_type;

  static_assert(std::is_same<container_buffer_sequence_type,
    buffer_sequence_type>::value, "Container buffer_sequence_type is not the "
      "same as MultipleBufferSequence buffer_sequence_type");

  static_assert(std::is_same<container_endpoint_type,
    endpoint_type>::value, "Container endpoint_type is not the same as "
      "MultipleBufferSequence buffer_sequence_type");

  static_assert(std::is_same<container_value_type,
    value_type>::value, "Container value_type is not the same as "
      "MultipleBufferSequence value_type");
#endif // defined(ASIO_HAS_STATIC_ASSERT)

public:
  typedef typename raw_container_type::size_type size_type;
  typedef typename raw_container_type::reference reference;
  typedef typename raw_container_type::const_reference const_reference;
  typedef typename raw_container_type::pointer pointer;
  typedef typename raw_container_type::const_pointer const_pointer;
  typedef typename raw_container_type::iterator iterator;
  typedef typename raw_container_type::const_iterator const_iterator;
  typedef typename raw_container_type::reverse_iterator reverse_iterator;
  typedef typename raw_container_type::const_reverse_iterator
    const_reverse_iterator;

protected:
  container_type container_;

protected:
  template <typename ReferenceContainer>
  explicit base_multiple_buffer_sequence_template(std::size_t offset,
    ReferenceContainer& reference_container)
    : base_type(offset), container_(reference_container)
  {
  }
  
public:
  base_multiple_buffer_sequence_template() = default;

  base_multiple_buffer_sequence_template(
    base_multiple_buffer_sequence_template const&) = default;
  
  base_multiple_buffer_sequence_template& operator = 
    (base_multiple_buffer_sequence_template const&) = default;

#if defined(ASIO_HAS_MOVE)
  base_multiple_buffer_sequence_template(
    base_multiple_buffer_sequence_template&&) = default;
  
  base_multiple_buffer_sequence_template& operator = 
    (base_multiple_buffer_sequence_template&&) = default;
#endif // defined(ASIO_HAS_MOVE)

  virtual ~base_multiple_buffer_sequence_template() ASIO_NOEXCEPT
  {
  }

  virtual std::size_t size() const ASIO_NOEXCEPT override
  {
    return container_.size();
  }

  virtual bool empty() const ASIO_NOEXCEPT override
  {
    return container_.empty();
  }

  virtual value_type& at(std::size_t index) override
  {
    return container_.at(index);
  }

  virtual const value_type& at(std::size_t index) const override
  {
    return container_.at(index);
  }

  reference front()
  {
    return container_.front();
  }

  const_reference front() const
  {
    return container_.front();
  }

  reference back()
  {
    return container_.back();
  }

  const_reference back() const
  {
    return container_.back();
  }

  pointer data() ASIO_NOEXCEPT
  {
    return container_.data();
  }

  const_pointer data() const ASIO_NOEXCEPT
  {
    return container_.data();
  }

  iterator begin() ASIO_NOEXCEPT
  {
    return container_.begin();
  }

  const_iterator begin() const ASIO_NOEXCEPT
  {
    return container_.begin();
  }

  const_iterator cbegin() const ASIO_NOEXCEPT
  {
    return container_.cbegin();
  }

  iterator end() ASIO_NOEXCEPT
  {
    return container_.end();
  }

  const_iterator end() const ASIO_NOEXCEPT
  {
    return container_.end();
  }

  const_iterator cend() const ASIO_NOEXCEPT
  {
    return container_.cend();
  }

  reverse_iterator rbegin() ASIO_NOEXCEPT
  {
    return container_.rbegin();
  }

  const_reverse_iterator rbegin() const ASIO_NOEXCEPT
  {
    return container_.rbegin();
  }

  const_reverse_iterator crbegin() const ASIO_NOEXCEPT
  {
    return container_.crbegin();
  }

  reverse_iterator rend() ASIO_NOEXCEPT
  {
    return container_.rend();
  }

  const_reverse_iterator rend() const ASIO_NOEXCEPT
  {
    return container_.rend();
  }

  const_reverse_iterator crend() const ASIO_NOEXCEPT
  {
    return container_.crend();
  }
};

template <typename BufferSequence, typename EndpointType, 
  std::size_t BufferSequenceCount>
class fixed_size_multiple_buffer_sequence
  : public base_multiple_buffer_sequence_template<BufferSequence,
    EndpointType, detail::array<detail::multiple_buffer_sequence_op<
    BufferSequence, EndpointType>, BufferSequenceCount>>
{
public:
  typedef base_multiple_buffer_sequence_template<BufferSequence,
    EndpointType, detail::array<detail::multiple_buffer_sequence_op<
    BufferSequence, EndpointType>, BufferSequenceCount>> base_type;

  typedef typename base_type::buffer_sequence_type buffer_sequence_type;
  typedef typename base_type::endpoint_type endpoint_type;
  typedef typename base_type::value_type value_type;

  typedef typename base_type::size_type size_type;
  typedef typename base_type::reference reference;
  typedef typename base_type::const_reference const_reference;
  typedef typename base_type::pointer pointer;
  typedef typename base_type::const_pointer const_pointer;
  typedef typename base_type::iterator iterator;
  typedef typename base_type::const_iterator const_iterator;
  typedef typename base_type::reverse_iterator reverse_iterator;
  typedef typename base_type::const_reverse_iterator const_reverse_iterator;

public:
  fixed_size_multiple_buffer_sequence() = default;

  fixed_size_multiple_buffer_sequence(
    const fixed_size_multiple_buffer_sequence&) = delete;

  fixed_size_multiple_buffer_sequence& operator = (
    const fixed_size_multiple_buffer_sequence&) = delete;

#if defined(ASIO_HAS_MOVE)
  fixed_size_multiple_buffer_sequence(
    fixed_size_multiple_buffer_sequence&&) = default;

  fixed_size_multiple_buffer_sequence& operator = (
    fixed_size_multiple_buffer_sequence&&) = default;
#endif // defined(ASIO_HAS_MOVE)

  explicit fixed_size_multiple_buffer_sequence(
    const buffer_sequence_type& buffer_sequence)
    : base_type()
  {
    this->at(0) = detail::multiple_buffer_sequence_op(buffer_sequence);
  }

  explicit fixed_size_multiple_buffer_sequence(
    const buffer_sequence_type& buffer_sequence,
    const endpoint_type& endpoint)
    : base_type()
  {
    this->at(0) = detail::multiple_buffer_sequence_op(buffer_sequence, endpoint);
  }
};

template <typename BufferSequence, typename EndpointType>
static inline fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType,
  1> make_fixed_size_multiple_buffer_sequence(
    const BufferSequence& buffer_sequence, const EndpointType& endpoint)
{
  return fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType, 1>(
    buffer_sequence, endpoint);
};

template <typename BufferSequence, typename EndpointType,
  typename ContainerAllocatorType = std::allocator<
  detail::multiple_buffer_sequence_op<BufferSequence, EndpointType>>>
class resizeable_multiple_buffer_sequence
  : public base_multiple_buffer_sequence_template<BufferSequence,
    EndpointType, std::vector<detail::multiple_buffer_sequence_op<
    BufferSequence, EndpointType>, ContainerAllocatorType>>
{
public:
  typedef base_multiple_buffer_sequence_template<BufferSequence,
    EndpointType, std::vector<detail::multiple_buffer_sequence_op<
    BufferSequence, EndpointType>, ContainerAllocatorType>> base_type;

  typedef typename base_type::buffer_sequence_type buffer_sequence_type;
  typedef typename base_type::endpoint_type endpoint_type;
  typedef typename base_type::value_type value_type;

  typedef ContainerAllocatorType container_allocator_type;

  typedef typename base_type::size_type size_type;
  typedef typename base_type::reference reference;
  typedef typename base_type::const_reference const_reference;
  typedef typename base_type::pointer pointer;
  typedef typename base_type::const_pointer const_pointer;
  typedef typename base_type::iterator iterator;
  typedef typename base_type::const_iterator const_iterator;
  typedef typename base_type::reverse_iterator reverse_iterator;
  typedef typename base_type::const_reverse_iterator const_reverse_iterator;

protected:
  void throw_out_of_range() const
  {
    throw std::out_of_range(
        "multiple buffer sequence size will be greather than maximum "
        "supported operations per io");
  }

public:
  resizeable_multiple_buffer_sequence() = default;

  resizeable_multiple_buffer_sequence(
    const resizeable_multiple_buffer_sequence&) = delete;

  resizeable_multiple_buffer_sequence& operator = (
    const resizeable_multiple_buffer_sequence&) = delete;

#if defined(ASIO_HAS_MOVE)
  resizeable_multiple_buffer_sequence(
    resizeable_multiple_buffer_sequence&&) = default;

  resizeable_multiple_buffer_sequence& operator = (
    resizeable_multiple_buffer_sequence&&) = default;
#endif // defined(ASIO_HAS_MOVE)

  explicit resizeable_multiple_buffer_sequence(
    const buffer_sequence_type& buffer_sequence)
    : base_type()
  {
    push_back(buffer_sequence);
  }

  explicit resizeable_multiple_buffer_sequence(
    const buffer_sequence_type& buffer_sequence,
    const endpoint_type& endpoint)
    : base_type()
  {
    push_back(buffer_sequence, endpoint);
  }

  void throw_if_overflow(size_type new_size) const
  {
    if (new_size > multiple_buffer_sequence_maximum_operations_per_io)
    {
      throw_out_of_range();
    }
  }

  void throw_if_full() const
  {
    if (this->size() == multiple_buffer_sequence_maximum_operations_per_io)
    {
      throw_out_of_range();
    }
  }
  
  void reserve(size_type count)
  {
    throw_if_overflow(count);

    this->container_.reserve(count);
  }

  size_type capacity() const ASIO_NOEXCEPT
  {
    return this->container_.capacity() > 
        multiple_buffer_sequence_maximum_operations_per_io ? 
        multiple_buffer_sequence_maximum_operations_per_io : 
        this->container_.capacity();
  }

  void shrink_to_fit() const
  {
    return this->container_.shrink_to_fit();
  }

  void clear()
  {
    this->container_.clear();
  }

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)
  template <typename... Args>
  void insert(const_iterator pos, Args&& ... args)
  {
    throw_if_full();
    
    this->container_.emplace(pos, value_type(std::forward<Args>(args)...));
  }

  template <typename... Args>
  void push_back(Args&& ... args)
  {
    throw_if_full();
    
    this->container_.emplace_back(value_type(std::forward<Args>(args)...));
  }
#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)
  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence)
  {
    throw_if_full();
    
    this->container_.insert(pos, value_type(buffer_sequence);
  }

  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    throw_if_full();
    
    this->container_.insert(pos, value_type(buffer_sequence, endpoint));
  }

  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint, socket_base::message_flags flags)
  {
    throw_if_full();
    
    this->container_.insert(pos, value_type(buffer_sequence, endpoint, flags));
  }

  void push_back(const_iterator pos,
      const buffer_sequence_type& buffer_sequence)
  {
    throw_if_full();
    
    this->container_.push_back(pos, value_type(buffer_sequence);
  }

  void push_back(const_iterator pos,
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    throw_if_full();
    
    this->container_.push_back(pos, value_type(buffer_sequence, endpoint));
  }

  void push_back(const_iterator pos, 
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint, socket_base::message_flags flags)
  {
    throw_if_full();

    this->container_.push_back(pos, value_type(buffer_sequence, endpoint, flags));
  }
#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  void erase(iterator pos)
  {
    this->container_.erase(pos);
  }

  void erase(const_iterator pos)
  {
    this->container_.erase(pos);
  }

  void erase(iterator first, iterator last)
  {
    this->container_.erase(first, last);
  }

  void erase(const_iterator first, const_iterator last)
  {
    this->container_.erase(first, last);
  }

  void pop_back()
  {
    this->container_.pop_back();
  }

  void resize(size_type count)
  {
    throw_if_overflow(count);

    size_type const previous_size = this->size();

    this->container_.resize(count);
  }
  
  size_type max_size() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_maximum_operations_per_io;
  }

  bool full() const ASIO_NOEXCEPT
  {
    return (this->size() >= max_size());
  }
  
  void swap(base_type& other)
  {
    std::swap(this->container_, other.container_);
    std::swap(this->offset_, other.offset_);
    std::swap(this->operations_executed_, other.operations_executed_);
    std::swap(this->bytes_transferred_, other.bytes_transferred_);
  }
};

template <typename MultipleBufferSequence>
class multiple_buffer_sequence_view
  : public base_multiple_buffer_sequence_template<typename
  MultipleBufferSequence::buffer_sequence_type, typename
  MultipleBufferSequence::endpoint_type, MultipleBufferSequence&>
{
public:
  typedef base_multiple_buffer_sequence_template<typename
    MultipleBufferSequence::buffer_sequence_type, typename
    MultipleBufferSequence::endpoint_type, MultipleBufferSequence&> base_type;

  typedef MultipleBufferSequence multiple_buffer_sequence_type;

  typedef typename multiple_buffer_sequence_type::buffer_sequence_type
    buffer_sequence_type;
  typedef typename multiple_buffer_sequence_type::endpoint_type endpoint_type;
  typedef typename multiple_buffer_sequence_type::value_type value_type;

  typedef typename multiple_buffer_sequence_type::size_type size_type;
  typedef typename multiple_buffer_sequence_type::reference reference;
  typedef typename multiple_buffer_sequence_type::const_reference
    const_reference;
  typedef typename multiple_buffer_sequence_type::pointer pointer;
  typedef typename multiple_buffer_sequence_type::const_pointer const_pointer;
  typedef typename multiple_buffer_sequence_type::iterator iterator;
  typedef typename multiple_buffer_sequence_type::const_iterator
    const_iterator;
  typedef typename multiple_buffer_sequence_type::reverse_iterator
    reverse_iterator;
  typedef typename multiple_buffer_sequence_type::const_reverse_iterator
    const_reverse_iterator;

public:
  multiple_buffer_sequence_view() = delete;

  explicit multiple_buffer_sequence_view(std::size_t offset, 
    multiple_buffer_sequence_type& multiple_buffer_sequence)
    : base_type(offset, multiple_buffer_sequence)
  {
  }

  multiple_buffer_sequence_view(
    const multiple_buffer_sequence_view&) = default;

  multiple_buffer_sequence_view& operator = (
    const multiple_buffer_sequence_view&) = default;

#if defined(ASIO_HAS_MOVE)
  multiple_buffer_sequence_view(
    multiple_buffer_sequence_view&&) = default;

  multiple_buffer_sequence_view& operator = (
    multiple_buffer_sequence_view&&) = default;
#endif // defined(ASIO_HAS_MOVE)
};

template <typename MultipleBufferSequence>
static inline multiple_buffer_sequence_view<MultipleBufferSequence>
  make_multiple_buffer_sequence_view(std::size_t offset,
    MultipleBufferSequence& sequence)
{
  return multiple_buffer_sequence_view<MultipleBufferSequence>(offset,
    sequence);
};

template <typename MultipleBufferSequence>
static inline multiple_buffer_sequence_view<const MultipleBufferSequence>
  make_multiple_buffer_sequence_view(std::size_t offset,
    const MultipleBufferSequence& sequence)
{
  return multiple_buffer_sequence_view<const MultipleBufferSequence>(offset,
    sequence);
};

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP
