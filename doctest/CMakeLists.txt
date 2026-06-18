include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    doctest 
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG master
)
FetchContent_GetProperties(doctest)
FetchContent_MakeAvailable(doctest) 