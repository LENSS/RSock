FIND_LIBRARY(Emon REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (ANDROID)
    link_directories("../staging_dir/lib")
endif()

add_subdirectory(proto)

set(SOURCE_FILES
        rm/RMCore.cpp
        rm/RMCore.h
        hrpNode.h
        mve/MVECore.cpp
        mve/MVECore.h
        hrpEexception.h
        mve/MVEEstimator.cpp
        mve/MVEEstimator.h
        rm/PILearner.cpp
        rm/PILearner.h
        rm/FILearner.cpp
        rm/FILearner.h
        common.h
        mve/DelayEstimator.cpp
        mve/DelayEstimator.h
        hrp/HrpCore.cpp
        hrp/HrpCore.h
        hrpMessages.h
        hrp/TxrxEngine.cpp
        hrp/TxrxEngine.h hrpNode.cpp
        hrp/MetaDataExchanger.h
        hrp/MetaDataExchanger.cpp
        hrp/HrpGraph.h
        hrp/HrpGraph.cpp hrp/TopologyManager.cpp hrp/TopologyManager.h hrp/HrpRouter.cpp hrp/HrpRouter.h hrpMessages.cpp api/TestAPI.cpp api/TestAPI.h api/HrpAPIServer.cpp api/HrpAPIServer.h api/HrpAPIPacket.cpp api/HrpAPIPacket.h daemon/daemon.h daemon/daemon.cpp)


if (ANDROID)
    list(APPEND SOURCE_FILES ifaddrs-android.cpp)
endif()
        

add_definitions(-DASIO_STANDALONE)

add_library(hrp SHARED ${SOURCE_FILES} api/HrpAPIClient.cpp api/HrpAPIClient.h)
target_link_libraries(hrp hrpproto emon ${CMAKE_THREAD_LIBS_INIT})

#add_library(hrpjni SHARED ${SOURCE_FILES} api/HrpAPIClient.cpp api/HrpAPIClient.h jni/main_JniTest.h jni/main_JniTest.cpp)
#target_link_libraries(hrpjni hrpproto emon)


add_executable(rsockd daemon/test_main.cpp ${SOURCE_FILES})
target_link_libraries(rsockd hrpproto emon ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_client daemon/test_client.cpp api/HrpAPIClient.h api/HrpAPIClient.cpp hrpNode.cpp api/HrpAPIPacket.cpp api/HrpAPIPacket.h)
target_link_libraries(test_client ${CMAKE_THREAD_LIBS_INIT})

# Added by Ala Altaweel to test GNS client
add_executable(test_gns daemon/test_gns.cpp gns/gns_service_client.h gns/gns_service_client.cpp)
target_link_libraries(test_gns ${CMAKE_THREAD_LIBS_INIT})
# done by Ala addings

install(TARGETS rsockd DESTINATION bin)
install(TARGETS test_client DESTINATION bin)
install(TARGETS hrp DESTINATION lib)
# Added by Ala Altaweel to test GNS client
install(TARGETS test_gns DESTINATION bin)
# done by Ala addings

#install(TARGETS hrpjni DESTINATION lib)

#add_executable(separate_testd daemon/test_main.cpp)
#target_link_libraries(separate_testd hrp)
#
#add_executable(testgraph daemon/test_graph.cpp)
#target_link_libraries(testgraph hrp)
#
#add_executable(testolsr daemon/test_olsr.cpp)
#target_link_libraries(testolsr hrp)
