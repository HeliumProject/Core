#pragma once

#include "Foundation/API.h"

namespace Helium
{
    /// Constant sparse array iterator.
    template< typename T >
    class ConstSparseArrayIterator
    {
    public:
        /// Type for iterator values.
        typedef T ValueType;

        /// Type for pointers to iterated elements.
        typedef T* PointerType;
        /// Type for references to iterated elements.
        typedef T& ReferenceType;
        /// Type for constant pointers to iterated elements.
        typedef const T* ConstPointerType;
        /// Type for constant references to iterated elements.
        typedef const T& ConstReferenceType;

        /// @name Construction/Destruction
        //@{
        ConstSparseArrayIterator();
        explicit ConstSparseArrayIterator(
            const T* pElement, const uint32_t* pUsedElementBits, uint32_t usedElementMask );
        //@}

        /// @name Overloaded Operators
        //@{
        operator bool() const;

        ConstReferenceType operator*() const;
        ConstPointerType operator->() const;

        ConstSparseArrayIterator& operator++();
        ConstSparseArrayIterator operator++( int );
        ConstSparseArrayIterator& operator--();
        ConstSparseArrayIterator operator--( int );

        ConstSparseArrayIterator& operator+=( ptrdiff_t offset );
        ConstSparseArrayIterator operator+( ptrdiff_t offset ) const;
        ConstSparseArrayIterator& operator-=( ptrdiff_t offset );
        ConstSparseArrayIterator operator-( ptrdiff_t offset ) const;

        ptrdiff_t operator-( const ConstSparseArrayIterator& rOther ) const;

        bool operator==( const ConstSparseArrayIterator& rOther ) const;
        bool operator!=( const ConstSparseArrayIterator& rOther ) const;
        bool operator<( const ConstSparseArrayIterator& rOther ) const;
        bool operator>( const ConstSparseArrayIterator& rOther ) const;
        bool operator<=( const ConstSparseArrayIterator& rOther ) const;
        bool operator>=( const ConstSparseArrayIterator& rOther ) const;
        //@}

    protected:
        /// Current array element.
        PointerType m_pElement;

        /// Current in-use bit array element.
        uint32_t* m_pUsedElementBits;
        /// Current in-use bit mask.
        uint32_t m_usedElementMask;
    };

    /// Non-constant sparse array iterator.
    template< typename T >
    class SparseArrayIterator : public ConstSparseArrayIterator< T >
    {
    public:
        /// Type for iterator values.
        typedef T ValueType;

        /// Type for pointers to iterated elements.
        typedef T* PointerType;
        /// Type for references to iterated elements.
        typedef T& ReferenceType;
        /// Type for constant pointers to iterated elements.
        typedef const T* ConstPointerType;
        /// Type for constant references to iterated elements.
        typedef const T& ConstReferenceType;

        /// @name Construction/Destruction
        //@{
        SparseArrayIterator();
        explicit SparseArrayIterator( T* pElement, uint32_t* pUsedElementBits, uint32_t usedElementMask );
        //@}

        /// @name Overloaded Operators
        //@{
        ReferenceType operator*() const;
        PointerType operator->() const;

        SparseArrayIterator& operator++();
        SparseArrayIterator operator++( int );
        SparseArrayIterator& operator--();
        SparseArrayIterator operator--( int );

        SparseArrayIterator& operator+=( ptrdiff_t offset );
        SparseArrayIterator operator+( ptrdiff_t offset ) const;
        SparseArrayIterator& operator-=( ptrdiff_t offset );
        SparseArrayIterator operator-( ptrdiff_t offset ) const;
        //@}
    };

    /// Resizable sparse array (not thread-safe).
    template< typename T, typename Allocator = DefaultAllocator >
    class SparseArray
    {
    public:
        /// Type for array element values.
        typedef T ValueType;

        /// Type for pointers to array elements.
        typedef T* PointerType;
        /// Type for references to array elements.
        typedef T& ReferenceType;
        /// Type for constant pointers to array elements.
        typedef const T* ConstPointerType;
        /// Type for constant references to array elements.
        typedef const T& ConstReferenceType;

        /// Iterator type.
        typedef SparseArrayIterator< T > Iterator;
        /// Constant iterator type.
        typedef ConstSparseArrayIterator< T > ConstIterator;

        typedef std::is_trivially_copy_assignable< T > TrivialCopyElements;
        typedef std::is_trivially_destructible< T > TrivialDestroyElements;

        /// @name Construction/Destruction
        //@{
        SparseArray();
        SparseArray( const T* pSource, size_t size );
        template< typename OtherAllocator > SparseArray( const SparseArray< T, OtherAllocator >& rSource );
        ~SparseArray();
        //@}

        /// @name Array Operations
        //@{
        size_t GetSize() const;
        size_t GetUsedSize() const;
        size_t GetCapacity() const;
        void Reserve( size_t capacity );
        void Trim();

        T* GetData();
        const T* GetData() const;

        const uint32_t* GetUsedElements() const;

        void Clear();

        Iterator Begin();
        ConstIterator Begin() const;
        Iterator End();
        ConstIterator End() const;

        bool IsElementValid( size_t index ) const;
        T& GetElement( size_t index );
        const T& GetElement( size_t index ) const;

        size_t GetElementIndex( const T& rElement ) const;
        size_t GetElementIndex( const T* pElement ) const;

        void Set( const T* pSource, size_t size );

        size_t Add( const T& rValue );
        void Remove( size_t index );

        void Swap( SparseArray& rArray );
        //@}

        /// @name In-place Object Creation
        //@{
        T* New();

        template <typename U>
        T* New(const U &u);

        template <typename U, typename V>
        T* New(const U &u, const V &v);
        //@}

        /// @name Overloaded Operators
        //@{
        SparseArray& operator=( const SparseArray& rSource );
        template< typename OtherAllocator > SparseArray& operator=( const SparseArray< T, OtherAllocator >& rSource );

        T& operator[]( ptrdiff_t index );
        const T& operator[]( ptrdiff_t index ) const;
        //@}

    private:
        /// Allocated array buffer.
        T* m_pBuffer;
        /// Bits specifying which array elements are in use.
        uint32_t* m_pUsedElements;

        /// Used buffer range.
        size_t m_size;
        /// Actual number of elements used.
        size_t m_usedSize;
        /// Buffer capacity.
        size_t m_capacity;

        /// Index of the lowest element last modified (searches for free slots will start from here when adding).
        size_t m_lastUpdateIndex;

        /// Allocator instance.
        Allocator m_allocator;

        /// @name Private Utility Functions
        //@{
        size_t GetGrowCapacity( size_t desiredCount ) const;
        void Grow( size_t capacity );

        size_t AllocateSlot();

        template< typename OtherAllocator > SparseArray& Assign( const SparseArray< T, OtherAllocator >& rSource );

        T* Allocate( size_t count );
        T* Allocate( size_t count, const std::true_type& rNeedsAlignment );
        T* Allocate( size_t count, const std::false_type& rNeedsAlignment );

        T* Reallocate( T* pMemory, size_t count );
        T* Reallocate( T* pMemory, size_t count, const std::true_type& rNeedsAlignment );
        T* Reallocate( T* pMemory, size_t count, const std::false_type& rNeedsAlignment );

		void Free( T* pMemory );
		void Free( T* pMemory, const std::true_type& rNeedsAlignment );
		void Free( T* pMemory, const std::false_type& rNeedsAlignment );

        T* ResizeBuffer( T* pMemory, size_t elementRange, const uint32_t* pUsedElements, size_t oldCapacity, size_t newCapacity );
        T* ResizeBuffer( T* pMemory, size_t elementRange, const uint32_t* pUsedElements, size_t oldCapacity, size_t newCapacity, const std::true_type& rHasTrivialCopyAndDestructor );
        T* ResizeBuffer( T* pMemory, size_t elementRange, const uint32_t* pUsedElements, size_t oldCapacity, size_t newCapacity, const std::false_type& rHasTrivialCopyAndDestructor );

        void InPlaceDestroy( T* pMemory, size_t range, const uint32_t* pUsedElements );
        void InPlaceDestroy( T* pMemory, size_t range, const uint32_t* pUsedElements, const std::true_type& rHasTrivialDestructor );
        void InPlaceDestroy( T* pMemory, size_t range, const uint32_t* pUsedElements, const std::false_type& rHasTrivialDestructor );

        void UninitializedCopy( T* pDest, const T* pSource, size_t range, const uint32_t* pUsedElements );
        void UninitializedCopy( T* pDest, const T* pSource, size_t range, const uint32_t* pUsedElements, const std::true_type& rHasTrivialCopy );
        void UninitializedCopy( T* pDest, const T* pSource, size_t range, const uint32_t* pUsedElements, const std::false_type& rHasTrivialCopy );
        //@}
    };
}

#include "Foundation/SparseArray.inl"
