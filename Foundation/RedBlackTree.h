#pragma once

#include "Platform/Trace.h"

#include "Foundation/BitArray.h"
#include "Foundation/DynamicArray.h"
#include "Foundation/Pair.h"
#include "Foundation/Functions.h"

namespace Helium
{
    /// Red-black tree implementation, backed by a dynamic array.
    template<
        typename Value, typename Key, typename ExtractKey, typename CompareKey = Less< Key >,
        typename Allocator = DefaultAllocator, typename InternalValue = Value >
    class RedBlackTree
    {
    private:
        struct LinkData;

    public:
        /// Type for tree keys.
        typedef Key KeyType;
        /// Type for tree entries.
        typedef Value ValueType;

        /// Internal value type (type used for actual value storage).
        typedef InternalValue InternalValueType;

        /// Type for comparing two keys.
        typedef CompareKey KeyCompareType;
        /// Allocator type.
        typedef Allocator AllocatorType;

        /// Constant red-black tree iterator.
        class ConstIterator
        {
            friend class RedBlackTree;

        public:
            // STL iterator support.
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef Value value_type;
            typedef ptrdiff_t difference_type;

            typedef const Value* pointer;
            typedef const Value& reference;

            /// @name Construction/Destruction
            //@{
            ConstIterator();
            //@}

            /// @name Overloaded Operators
            //@{
            const Value& operator*() const;
            const Value* operator->() const;

            ConstIterator& operator++();
            ConstIterator operator++( int );
            ConstIterator& operator--();
            ConstIterator operator--( int );

            bool operator==( const ConstIterator& rOther ) const;
            bool operator!=( const ConstIterator& rOther ) const;
            bool operator<( const ConstIterator& rOther ) const;
            bool operator>( const ConstIterator& rOther ) const;
            bool operator<=( const ConstIterator& rOther ) const;
            bool operator>=( const ConstIterator& rOther ) const;
            //@}

        protected:
            /// Tree instance.
            RedBlackTree* m_pTree;
            /// Current tree node index.
            size_t m_index;

            /// @name Construction/Destruction, Protected
            //@{
            ConstIterator( const RedBlackTree* pTree, size_t index );
            //@}
        };

        /// Red-black tree iterator.
        class Iterator : public ConstIterator
        {
            friend class RedBlackTree;

        public:
            // STL iterator support.
            typedef typename ConstIterator::iterator_category iterator_category;
            typedef typename ConstIterator::value_type value_type;
            typedef typename ConstIterator::difference_type difference_type;

            typedef Value* pointer;
            typedef Value& reference;

            /// @name Construction/Destruction
            //@{
            Iterator();
            //@}

            /// @name Overloaded Operators
            //@{
            Value& operator*() const;
            Value* operator->() const;

            Iterator& operator++();
            Iterator operator++( int );
            Iterator& operator--();
            Iterator operator--( int );
            //@}

        protected:
            /// @name Construction/Destruction, Protected
            //@{
            Iterator( RedBlackTree* pTree, size_t index );
            //@}
        };

        /// @name Construction/Destruction
        //@{
        RedBlackTree();
        RedBlackTree( const RedBlackTree& rSource );
        template< typename OtherAllocator > RedBlackTree(
            const RedBlackTree< Value, Key, ExtractKey, CompareKey, OtherAllocator, InternalValue >& rSource );
        //@}

        /// @name Tree Operations
        //@{
        size_t GetSize() const;
        bool IsEmpty() const;

        size_t GetCapacity() const;
        void Reserve( size_t capacity );
        void Trim();

        void Clear();

        Iterator Begin();
        ConstIterator Begin() const;
        Iterator End();
        ConstIterator End() const;

        Iterator Find( const Key& rKey );
        ConstIterator Find( const Key& rKey ) const;

        Pair< Iterator, bool > Insert( const Value& rValue );
        bool Insert( ConstIterator& rIterator, const Value& rValue );

        bool Remove( const Key& rKey );
        void Remove( Iterator iterator );

        void Swap( RedBlackTree& rTree );
        //@}

        /// @name Debug Verification
        //@{
        bool Verify() const;
        //@}

        /// @name Overloaded Operators
        //@{
        RedBlackTree& operator=( const RedBlackTree& rSource );
        template< typename OtherAllocator > RedBlackTree& operator=(
            const RedBlackTree< Value, Key, ExtractKey, CompareKey, OtherAllocator, InternalValue >& rSource );
        //@}

    private:
        /// Node link data.
        struct LinkData
        {
            /// Parent node index.
            size_t parent;
            /// Child node indices (0 = left, 1 = right).
            size_t children[ 2 ];
        };

        /// Tree node values.
        DynamicArray< InternalValue, Allocator > m_values;
        /// Tree node link data.
        DynamicArray< LinkData, Allocator > m_links;
        /// Red-black state bits for each node (set = black, clear = red).
        BitArray< Allocator > m_blackNodes;

        /// Root node index.
        size_t m_root;

        /// @name Private Utility Functions
        //@{
        template< typename OtherAllocator > void Copy(
            const RedBlackTree< Value, Key, ExtractKey, CompareKey, OtherAllocator, InternalValue >& rSource );

        size_t FindNodeIndex( const Key& rKey ) const;

        size_t FindFirstNodeIndex() const;
        size_t FindLastNodeIndex() const;

        size_t RotateNode( size_t nodeIndex, size_t childLinkIndex );

        size_t RecursiveVerify( size_t nodeIndex ) const;
        //@}
    };
}

#include "Foundation/RedBlackTree.inl"
