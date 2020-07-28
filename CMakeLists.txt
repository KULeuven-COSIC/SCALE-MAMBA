cmake_minimum_required(VERSION 3.1...3.14)

if (${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif ()

project(SCALE_MAMBA VERSION 1.8 DESCRIPTION "SCALE MAMBA" LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 11)

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

include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src ${MPIRXX_INCLUDE_DIRS})

add_library(exceptions OBJECT
        src/Exceptions/handler.cpp
        )

add_library(fhe OBJECT
        src/FHE/Ciphertext.cpp
        src/FHE/Distributions.cpp
        src/FHE/FFT.cpp
        src/FHE/FFT_Data.cpp
        src/FHE/FHE_Keys.cpp
        src/FHE/FHE_Params.cpp
        src/FHE/Plaintext.cpp
        src/FHE/Random_Coins.cpp
        src/FHE/Ring.cpp
        src/FHE/Ring_Element.cpp
        src/FHE/Rq_Element.cpp
        src/FHE/ZKPoK.cpp
        )

add_library(gc OBJECT
        src/GC/Base_Circuits.cpp
        src/GC/Circuit.cpp
        src/GC/Garbled.cpp
        src/GC/SimplifyCircuit.cpp
        )

add_library(io OBJECT
        src/Input_Output/Input_Output_Base.cpp
        src/Input_Output/Input_Output_Simple.cpp
        )

add_library(lsss OBJECT
        src/LSSS/CAS.cpp
        src/LSSS/MSP.cpp
        src/LSSS/Open_Protocol.cpp
        src/LSSS/PRSS.cpp
        src/LSSS/PRZS.cpp
        src/LSSS/Schur_Matrices.cpp
        src/LSSS/Share.cpp
        src/LSSS/ShareData.cpp
        )

add_library(mpcmath OBJECT
        src/Math/bigint.cpp
        src/Math/gf2n.cpp
        src/Math/gfp.cpp
        src/Math/Integer.cpp
        src/Math/Matrix.cpp
        src/Math/modp.cpp
        src/Math/Zp_Data.cpp
        )

add_library(offline OBJECT
        src/Offline/DABitGenerator.cpp
        src/Offline/DABitMachine.cpp
        src/Offline/FakePrep.cpp
        src/Offline/FHE_Factory.cpp
        src/Offline/offline.cpp
        src/Offline/offline_data.cpp
        src/Offline/offline_FHE.cpp
        src/Offline/offline_IO_production.cpp
        src/Offline/offline_Maurer.cpp
        src/Offline/offline_phases.cpp
        src/Offline/offline_Reduced.cpp
        src/Offline/offline_subroutines.cpp
        src/Offline/sacrifice.cpp
        src/Offline/XOR_Machine.cpp
        )

add_library(online OBJECT
        src/Online/Machine.cpp
        src/Online/Online.cpp
        src/Online/Schedule.cpp
        )

add_library(ot OBJECT
        src/OT/aAND.cpp
        src/OT/aAND_Thread.cpp
        src/OT/aBit.cpp
        src/OT/aBit_Thread.cpp
        src/OT/aBitFactory.cpp
        src/OT/aBitVector.cpp
        src/OT/BitMatrix.cpp
        src/OT/BitVector.cpp
        src/OT/COT.cpp
        src/OT/CRS.cpp
        src/OT/DMC.cpp
        src/OT/HaAND.cpp
        src/OT/LaAND.cpp
        src/OT/ROT.cpp
        src/OT/SimpleROT.cpp
        )
target_link_libraries(ot PRIVATE ${CRYPTOPP_LIBRARIES})
target_include_directories(ot PRIVATE ${CRYPTOPP_INCLUDE_DIRS})

add_library(processor OBJECT
        src/Processor/Instruction.cpp
        src/Processor/Memory.cpp
        src/Processor/Processor.cpp
        src/Processor/Processor_IO.cpp
        src/Processor/Program.cpp
        )

add_library(system OBJECT
        src/System/Networking.cpp
        src/System/Player.cpp
        src/System/RunTime.cpp
        src/System/SystemData.cpp
        )
target_link_libraries(system PRIVATE OpenSSL::SSL)

add_library(tools OBJECT
        src/Tools/aes-ni.cpp
        src/Tools/aes.cpp
        src/Tools/CBC-MAC.cpp
        src/Tools/Crypto.cpp
        src/Tools/MMO.cpp
        src/Tools/random.cpp
        src/Tools/Timer.cpp
        src/Tools/util_containers.cpp
        )

add_library(local OBJECT
        src/Local/BLAS.cpp
        src/Local/Local_Functions.cpp
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

add_executable(Player.x src/Player.cpp)
target_link_libraries(Player.x PRIVATE scale)

add_executable(Setup.x src/Setup.cpp)
target_link_libraries(Setup.x PRIVATE OpenSSL::Crypto OpenSSL::SSL scale)

add_subdirectory(Test)
