#if HELIUM_ENABLE_TRACE

/// Get the current logging level.
///
/// @return  Current logging level.
///
/// @see SetLevel()
Helium::TraceLevel Helium::Trace::GetLevel()
{
    return sm_level;
}

#endif  // HELIUM_ENABLE_TRACE
