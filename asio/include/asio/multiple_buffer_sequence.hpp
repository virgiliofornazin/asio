//
// multiple_buffer_sequence.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO: update header with copyright of asio C++ library )
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
#include "asio/detail/multiple_buffer_sequence_operation.hpp"
#include "asio/socket_base.hpp"
#include <stdexcept>
#include <memory>
#include <iterator>
#include <array>
#include <vector>

#include "asio/detail/push_options.hpp"

namespace asio {

static const std::size_t multiple_buffer_sequence_maximum_operations_per_io = 1
#if defined(ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO)
    * ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO
#endif // defined(ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO)
;

template <typename BufferSequence, typename EndpointType,
    typename ContainerType>
class base_multiple_buffer_sequence
{
public:
  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType,
      ContainerType> this_type;

  typedef BufferSequence buffer_sequence_type;
  typedef EndpointType endpoint_type;
  typedef ContainerType container_type;

  typedef detail::multiple_buffer_sequence_operation<buffer_sequence_type, 
      endpoint_type> value_type;

protected:
#if defined(ASIO_HAS_STATIC_ASSERT)
  typedef typename container_type::value_type container_value_type; 
  
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
  typedef typename container_type::size_type size_type;
  typedef typename container_type::reference reference;
  typedef typename container_type::const_reference const_reference;
  typedef typename container_type::pointer pointer;
  typedef typename container_type::const_pointer const_pointer;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::reverse_iterator reverse_iterator;
  typedef typename container_type::const_reverse_iterator
      const_reverse_iterator;

protected:
  container_type m_container;

protected:
  void throw_out_of_range() const
  {
    throw std::out_of_range(
        "multiple buffer sequence size will be greather than maximum "
        "supported operations per io");
  }

  void throw_empty() const
  {
    throw std::out_of_range(
        "no operations were assigned in multiple buffer sequence");
  }
  
public:
  base_multiple_buffer_sequence()
  {
  }

  base_multiple_buffer_sequence(const buffer_sequence_type& buffer_sequence)
  {
    push_back(buffer_sequence);
  }

  explicit base_multiple_buffer_sequence(
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    push_back(buffer_sequence, endpoint);
  }

  base_multiple_buffer_sequence(base_multiple_buffer_sequence const& other)
    : m_container(other.begin(), other.end())
  {
  }
  
#if defined(ASIO_HAS_MOVE)
  base_multiple_buffer_sequence(base_multiple_buffer_sequence&& other)
    : m_container(std::move(other))
  {
  }
#endif // defined(ASIO_HAS_MOVE)  

  base_multiple_buffer_sequence& operator = (
      base_multiple_buffer_sequence const& other)
  {
    m_container = other.m_container;
    return (*this);
  }
  
#if defined(ASIO_HAS_MOVE)
  base_multiple_buffer_sequence& operator = (
      base_multiple_buffer_sequence&& other)
  {
    m_container = std::move(other.m_container);
    return (*this);
  }
#endif // defined(ASIO_HAS_MOVE) 
  
  virtual ~base_multiple_buffer_sequence() ASIO_NOEXCEPT
  {
  }

  void reset()
  {
    size_type const op_count = size();

    for (size_t i = 0; i < op_count; ++i)
    {
      reference buffer_sequence = m_container.at(i);

      buffer_sequence.reset();
    }
  }

  void throw_if_empty() const
  {
    if (empty())
    {
      throw_empty();
    }
  }

  reference at(size_type index)
  {
    return m_container.at(index);
  }

  const_reference at(size_type index) const
  {
    return m_container.at(index);
  }

  reference operator[](std::size_t index)
  {
    return m_container[index];
  }

  const_reference operator[](std::size_t index) const
  {
    return m_container[index];
  }

  reference front()
  {
    return m_container.front();
  }

  const_reference front() const
  {
    return m_container.front();
  }

  reference back()
  {
    return m_container.back();
  }

  const_reference back() const
  {
    return m_container.back();
  }

  pointer data() ASIO_NOEXCEPT
  {
    return m_container.data();
  }

  const_pointer data() const ASIO_NOEXCEPT
  {
    return m_container.data();
  }

  iterator begin() ASIO_NOEXCEPT
  {
    return m_container.begin();
  }

  const_iterator begin() const ASIO_NOEXCEPT
  {
    return m_container.begin();
  }

  const_iterator cbegin() const ASIO_NOEXCEPT
  {
    return m_container.cbegin();
  }

  iterator end() ASIO_NOEXCEPT
  {
    return m_container.end();
  }

  const_iterator end() const ASIO_NOEXCEPT
  {
    return m_container.end();
  }

  const_iterator cend() const ASIO_NOEXCEPT
  {
    return m_container.cend();
  }

  reverse_iterator rbegin() ASIO_NOEXCEPT
  {
    return m_container.rbegin();
  }

  const_reverse_iterator rbegin() const ASIO_NOEXCEPT
  {
    return m_container.rbegin();
  }

  const_reverse_iterator crbegin() const ASIO_NOEXCEPT
  {
    return m_container.crbegin();
  }

  reverse_iterator rend() ASIO_NOEXCEPT
  {
    return m_container.rend();
  }

  const_reverse_iterator rend() const ASIO_NOEXCEPT
  {
    return m_container.rend();
  }

  const_reverse_iterator crend() const ASIO_NOEXCEPT
  {
    return m_container.crend();
  }

  bool empty() const ASIO_NOEXCEPT
  {
    return m_container.empty();
  }

  size_type size() const ASIO_NOEXCEPT
  {
    return m_container.size();
  }

  size_type max_size() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_maximum_operations_per_io;
  }

  void swap(base_multiple_buffer_sequence<BufferSequence, EndpointType, 
      ContainerType>& other)
  {
    std::swap(m_container, other.m_container);
  }

  bool all_empty() const
  {
    if (empty())
    {
      return true;
    }

    size_type const op_count = size();

    for (size_t i = 0; i < op_count; ++i)
    {
      const_reference buffer_sequence = at(i);

      if (!buffer_sequence.all_empty())
      {
        return false;
      }
    }
    
    return true;
  }

  bool full() const ASIO_NOEXCEPT
  {
    return (size() >= max_size());
  }
};

template <typename BufferSequence, typename EndpointType, 
    std::size_t BufferSequenceCount>
class fixed_size_multiple_buffer_sequence
  : public base_multiple_buffer_sequence<BufferSequence, EndpointType,
  std::array<detail::multiple_buffer_sequence_operation<BufferSequence, EndpointType>, 
    BufferSequenceCount>>
{
public:
  typedef fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType,
      BufferSequenceCount> this_type;

  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType,
      std::array<detail::multiple_buffer_sequence_operation<BufferSequence,
      EndpointType>, BufferSequenceCount>> base_type;

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
    typename BufferSequenceContainerAllocatorType = std::allocator<
    detail::multiple_buffer_sequence_operation<BufferSequence, EndpointType>>>
class resizeable_multiple_buffer_sequence
  : public base_multiple_buffer_sequence<BufferSequence, EndpointType,
  std::vector<detail::multiple_buffer_sequence_operation<BufferSequence, 
    EndpointType>, BufferSequenceContainerAllocatorType>>
{
public:
  typedef resizeable_multiple_buffer_sequence<BufferSequence, EndpointType,
      BufferSequenceContainerAllocatorType> this_type;

  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType,
      std::vector<detail::multiple_buffer_sequence_operation<BufferSequence, 
      EndpointType>, BufferSequenceContainerAllocatorType>> base_type;

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

protected:
  void throw_if_overflow(size_type new_size) const
  {
    if (new_size > multiple_buffer_sequence_maximum_operations_per_io)
    {
      this->throw_out_of_range();
    }
  }

  void throw_if_full() const
  {
    if (this->size() == multiple_buffer_sequence_maximum_operations_per_io)
    {
      this->throw_out_of_range();
    }
  }

public:
  void reserve(size_type count)
  {
    throw_if_overflow(count);

    this->m_container.reserve(count);
  }

  size_type capacity() const ASIO_NOEXCEPT
  {
    return this->m_container.capacity() > 
        multiple_buffer_sequence_maximum_operations_per_io ? 
        multiple_buffer_sequence_maximum_operations_per_io : 
        this->m_container.capacity();
  }

  void shrink_to_fit() const
  {
    return this->m_container.shrink_to_fit();
  }

  void clear()
  {
    this->m_container.clear();
  }

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)
  template <typename... Args>
  void insert(const_iterator pos, Args&& ... args)
  {
    throw_if_full();
    
    this->m_container.emplace(pos, value_type(std::forward<Args>(args)...));
  }

  template <typename... Args>
  void push_back(Args&& ... args)
  {
    throw_if_full();
    
    this->m_container.emplace_back(value_type(std::forward<Args>(args)...));
  }
#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)
  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence)
  {
    throw_if_full();
    
    this->m_container.insert(pos, value_type(buffer_sequence);
  }

  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    throw_if_full();
    
    this->m_container.insert(pos, value_type(buffer_sequence, endpoint));
  }

  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint, socket_base::message_flags flags)
  {
    throw_if_full();
    
    this->m_container.insert(pos, value_type(buffer_sequence, endpoint, flags));
  }

  void push_back(const_iterator pos,
      const buffer_sequence_type& buffer_sequence)
  {
    throw_if_full();
    
    this->m_container.push_back(pos, value_type(buffer_sequence);
  }

  void push_back(const_iterator pos,
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    throw_if_full();
    
    this->m_container.push_back(pos, value_type(buffer_sequence, endpoint));
  }

  void push_back(const_iterator pos, 
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint, socket_base::message_flags flags)
  {
    throw_if_full();

    this->m_container.push_back(pos, value_type(buffer_sequence, endpoint, flags));
  }
#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  void erase(iterator pos)
  {
    return this->m_container.erase(pos);
  }

  void erase(const_iterator pos)
  {
    return this->m_container.erase(pos);
  }

  void erase(iterator first, iterator last)
  {
    return this->m_container.erase(first, last);
  }

  void erase(const_iterator first, const_iterator last)
  {
    return this->m_container.erase(first, last);
  }

  void pop_back()
  {
    this->m_container.pop_back();
  }

  void resize(size_type count)
  {
    throw_if_overflow(count);

    size_type const previous_size = this->size();

    this->m_container.resize(count);
  }
};

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP
