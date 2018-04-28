/// Constructor
template< typename Key, typename CompareKey, typename Allocator >
Helium::SortedSet< Key, CompareKey, Allocator >::SortedSet()
{
}

/// Copy constructor.
///
/// @param[in] rSource  Source object from which to copy.
template< typename Key, typename CompareKey, typename Allocator >
Helium::SortedSet< Key, CompareKey, Allocator >::SortedSet( const SortedSet& rSource )
    : Base( rSource )
{
}

/// Copy constructor.
///
/// @param[in] rSource  Source object from which to copy.
template< typename Key, typename CompareKey, typename Allocator >
template< typename OtherAllocator >
Helium::SortedSet< Key, CompareKey, Allocator >::SortedSet(
    const SortedSet< Key, CompareKey, OtherAllocator >& rSource )
    : Base( rSource )
{
}

/// Assignment operator.
///
/// @param[in] rSource  Source object from which to copy.
///
/// @return  Reference to this object.
template< typename Key, typename CompareKey, typename Allocator >
Helium::SortedSet< Key, CompareKey, Allocator >& Helium::SortedSet< Key, CompareKey, Allocator >::operator=(
    const SortedSet& rSource )
{
    Base::operator=( rSource );

    return *this;
}

/// Assignment operator.
///
/// @param[in] rSource  Source object from which to copy.
///
/// @return  Reference to this object.
template< typename Key, typename CompareKey, typename Allocator >
template< typename OtherAllocator >
Helium::SortedSet< Key, CompareKey, Allocator >& Helium::SortedSet< Key, CompareKey, Allocator >::operator=(
    const SortedSet< Key, CompareKey, OtherAllocator >& rSource )
{
    Base::operator=( rSource );

    return *this;
}

/// Equality comparison operator.
///
/// @param[in] rOther  Set with which to compare.
///
/// @return  True if this set and the given set match, false if they differ.
///
/// @see operator!=()
template< typename Key, typename CompareKey, typename Allocator >
bool Helium::SortedSet< Key, CompareKey, Allocator >::operator==( const SortedSet& rOther ) const
{
    return Equals( rOther );
}

/// Equality comparison operator.
///
/// @param[in] rOther  Set with which to compare.
///
/// @return  True if this set and the given set match, false if they differ.
///
/// @see operator!=()
template< typename Key, typename CompareKey, typename Allocator >
template< typename OtherAllocator >
bool Helium::SortedSet< Key, CompareKey, Allocator >::operator==(
    const SortedSet< Key, CompareKey, OtherAllocator >& rOther ) const
{
    return Equals( rOther );
}

/// Inequality comparison operator.
///
/// @param[in] rOther  Set with which to compare.
///
/// @return  True if this set and the given set differ, false if they match.
///
/// @see operator==()
template< typename Key, typename CompareKey, typename Allocator >
bool Helium::SortedSet< Key, CompareKey, Allocator >::operator!=( const SortedSet& rOther ) const
{
    return !Equals( rOther );
}

/// Inequality comparison operator.
///
/// @param[in] rOther  Set with which to compare.
///
/// @return  True if this set and the given set differ, false if they match.
///
/// @see operator==()
template< typename Key, typename CompareKey, typename Allocator >
template< typename OtherAllocator >
bool Helium::SortedSet< Key, CompareKey, Allocator >::operator!=(
    const SortedSet< Key, CompareKey, OtherAllocator >& rOther ) const
{
    return !Equals( rOther );
}

/// Test whether the contents of this set match those of a given set.
///
/// @param[in] rOther  Set with which to compare.
///
/// @return  True if this set and the given set match, false if they differ.
template< typename Key, typename CompareKey, typename Allocator >
template< typename OtherAllocator >
bool Helium::SortedSet< Key, CompareKey, Allocator >::Equals(
    const SortedSet< Key, CompareKey, OtherAllocator >& rOther ) const
{
    if( Base::GetSize() != rOther.GetSize() )
    {
        return false;
    }

    typename Base::ConstIterator thisIter = Base::Begin();
    typename Base::ConstIterator thisEnd = Base::End();

    typename Base::ConstIterator otherIter = rOther.Begin();
    typename Base::ConstIterator otherEnd = rOther.End();

    CompareKey keyCompare;

    for( ; thisIter != thisEnd; ++thisIter, ++otherIter )
    {
        HELIUM_ASSERT( otherIter != otherEnd );

        const Key& rThisKey = *thisIter;
        const Key& rOtherKey = *otherIter;

        if( keyCompare( rThisKey, rOtherKey ) || keyCompare( rOtherKey, rThisKey ) )
        {
            return false;
        }
    }

    return true;
}
