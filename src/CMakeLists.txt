cmake_minimum_required(VERSION 3.1...3.14)

if (${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif ()

project(SCALE_MAMBA VERSION 1.0 DESCRIPTION "SCALE MAMBA" LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 11)

include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# TODO is it possible to make this into a runtime parameter?
# TODO why is it not in config.h as well?
add_definitions(-DMAX_MOD_SZ=7)
add_compile_options(
        -maes
        -mpclmul
        -msse4.1
        -mavx
        -march=core2
)

find_package(MPIR REQUIRED)
find_package(MPIRXX REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CryptoPP REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(exceptions OBJECT
        Exceptions/handler.cpp
        )

add_library(fhe OBJECT
        FHE/Ciphertext.cpp
        FHE/Distributions.cpp
        FHE/FFT.cpp
        FHE/FFT_Data.cpp
        FHE/FHE_Keys.cpp
        FHE/FHE_Params.cpp
        FHE/Plaintext.cpp
        FHE/Random_Coins.cpp
        FHE/Ring.cpp
        FHE/Ring_Element.cpp
        FHE/Rq_Element.cpp
        FHE/ZKPoK.cpp
        )

add_library(gc OBJECT
        GC/Base_Circuits.cpp
        GC/Circuit.cpp
        GC/Garbled.cpp
        GC/SimplifyCircuit.cpp
        )

add_library(io OBJECT
        Input_Output/Input_Output_Base.cpp
        Input_Output/Input_Output_Simple.cpp
        )

add_library(lsss OBJECT
        LSSS/CAS.cpp
        LSSS/MSP.cpp
        LSSS/Open_Protocol.cpp
        LSSS/PRSS.cpp
        LSSS/PRZS.cpp
        LSSS/Schur_Matrices.cpp
        LSSS/Share.cpp
        LSSS/ShareData.cpp
        )

add_library(mpcmath OBJECT
        Math/bigint.cpp
        Math/gf2n.cpp
        Math/gfp.cpp
        Math/Integer.cpp
        Math/Matrix.cpp
        Math/modp.cpp
        Math/Zp_Data.cpp
        )

add_library(offline OBJECT
        Offline/DABitGenerator.cpp
        Offline/DABitMachine.cpp
        Offline/FakePrep.cpp
        Offline/FHE_Factory.cpp
        Offline/offline.cpp
        Offline/offline_data.cpp
        Offline/offline_FHE.cpp
        Offline/offline_IO_production.cpp
        Offline/offline_Maurer.cpp
        Offline/offline_phases.cpp
        Offline/offline_Reduced.cpp
        Offline/offline_subroutines.cpp
        Offline/sacrifice.cpp
        Offline/XOR_Machine.cpp
        )

add_library(online OBJECT
        Online/Machine.cpp
        Online/Online.cpp
        Online/Schedule.cpp
        )

add_library(ot OBJECT
        OT/aAND.cpp
        OT/aAND_Thread.cpp
        OT/aBit.cpp
        OT/aBit_Thread.cpp
        OT/aBitFactory.cpp
        OT/aBitVector.cpp
        OT/BitMatrix.cpp
        OT/BitVector.cpp
        OT/COT.cpp
        OT/CRS.cpp
        OT/DMC.cpp
        OT/HaAND.cpp
        OT/LaAND.cpp
        OT/ROT.cpp
        OT/SimpleROT.cpp
        )
target_link_libraries(ot PRIVATE ${CRYPTOPP_LIBRARIES})
target_include_directories(ot PRIVATE ${CRYPTOPP_INCLUDE_DIRS})

add_library(processor OBJECT
        Processor/Instruction.cpp
        Processor/Memory.cpp
        Processor/Processor.cpp
        Processor/Processor_IO.cpp
        Processor/Program.cpp
        )

add_library(system OBJECT
        System/Networking.cpp
        System/Player.cpp
        System/RunTime.cpp
        System/SystemData.cpp
        )
target_link_libraries(system PRIVATE OpenSSL::SSL)

add_library(tools OBJECT
        Tools/aes-ni.cpp
        Tools/aes.cpp
        Tools/CBC-MAC.cpp
        Tools/Crypto.cpp
        Tools/MMO.cpp
        Tools/random.cpp
        Tools/Timer.cpp
        Tools/util_containers.cpp
        )

add_library(local OBJECT
        Local/BLAS.cpp
        Local/Local_Functions.cpp
        )

add_library(scale
        $<TARGET_OBJECTS:exceptions>
        $<TARGET_OBJECTS:fhe>
        $<TARGET_OBJECTS:gc>
        $<TARGET_OBJECTS:io>
        $<TARGET_OBJECTS:local>
        $<TARGET_OBJECTS:lsss>
        $<TARGET_OBJECTS:mpcmath>
        $<TARGET_OBJECTS:offline>
        $<TARGET_OBJECTS:online>
        $<TARGET_OBJECTS:ot>
        $<TARGET_OBJECTS:processor>
        $<TARGET_OBJECTS:system>
        $<TARGET_OBJECTS:tools>
        )
target_link_libraries(scale PRIVATE
        Threads::Threads OpenSSL::Crypto OpenSSL::SSL
        ${CRYPTOPP_LIBRARIES} ${MPIR_LIBRARIES} ${MPIRXX_LIBRARIES}
        )

add_executable(player Player.cpp)
target_link_libraries(player PRIVATE scale)

add_executable(setup Setup.cpp)
target_link_libraries(setup PRIVATE OpenSSL::Crypto OpenSSL::SSL scale)
