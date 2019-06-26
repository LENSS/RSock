FIND_LIBRARY(Emon REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (ANDROID)
    link_directories("../staging_dir/lib")
endif()

add_subdirectory(proto)

set(SOURCE_FILES
        common.h
        rm/RMCore.cpp
        rm/RMCore.h
        hrpNode.h
        hrpNode.cpp
        mve/MVECore.cpp
        mve/MVECore.h
        hrpEexception.h
        mve/MVEEstimator.cpp
        mve/MVEEstimator.h
        rm/PILearner.cpp
        rm/PILearner.h
        rm/FILearner.cpp
        rm/FILearner.h
        mve/DelayEstimator.cpp
        mve/DelayEstimator.h
        hrp/HrpCore.cpp
        hrp/HrpCore.h
        hrpMessages.h
        hrp/TxrxEngine.cpp
        hrp/TxrxEngine.h 
        hrp/MetaDataExchanger.h
        hrp/MetaDataExchanger.cpp
        hrp/HrpGraph.h
        hrp/HrpGraph.cpp
	hrp/TopologyManager.cpp
	hrp/TopologyManager.h
	hrp/HrpRouter.cpp
	hrp/HrpRouter.h
	hrpMessages.cpp
	api/TestAPI.cpp
	api/TestAPI.h
	api/HrpAPIServer.cpp
	api/HrpAPIServer.h
	api/HrpAPIPacket.cpp
	api/HrpAPIPacket.h
	gns/GnsServiceClient.cpp
	gns/GnsServiceClient.h
	api/HrpAPIClient.cpp
	api/HrpAPIClient.h
	daemon/daemon.h
	daemon/daemon.cpp)


if (ANDROID)
    list(APPEND SOURCE_FILES ifaddrs-android.cpp)
endif()
        


add_definitions(-DASIO_STANDALONE)

add_library(hrp SHARED ${SOURCE_FILES})
target_link_libraries(hrp hrpproto emon ${CMAKE_THREAD_LIBS_INIT})

add_executable(rsockd daemon/test_main.cpp ${SOURCE_FILES})
target_link_libraries(rsockd hrpproto emon ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_client daemon/test_client.cpp ${SOURCE_FILES})
target_link_libraries(test_client hrpproto ${CMAKE_THREAD_LIBS_INIT})

# Added by Ala Altaweel to test GNS client
add_executable(test_gns daemon/test_gns.cpp ${SOURCE_FILES})
target_link_libraries(test_gns hrpproto ${CMAKE_THREAD_LIBS_INIT})


install(TARGETS rsockd DESTINATION bin)
install(TARGETS hrp DESTINATION lib)

install(TARGETS test_client DESTINATION bin)
# Added by Ala Altaweel to test GNS client
install(TARGETS test_gns DESTINATION bin)	




#add_executable(separate_testd daemon/test_main.cpp)
#target_link_libraries(separate_testd hrp)
#
#add_executable(testgraph daemon/test_graph.cpp)
#target_link_libraries(testgraph hrp)
#
#add_executable(testolsr daemon/test_olsr.cpp)
#target_link_libraries(testolsr hrp)
