cmake_minimum_required(VERSION 3.12)
Project(Hive_Service)

set(THRIFT_COMPILER thrift)
set(THRIFT_GEN_CXX "cpp:templates,pure_enums,no_skeleton,include_prefix")

function(thrift_gen lang thrift_file output_dir)
  set(thrift_file_abs "${CMAKE_CURRENT_SOURCE_DIR}/${thrift_file}")
  set(output_dir_abs "${CMAKE_CURRENT_SOURCE_DIR}/${output_dir}")
  if (EXISTS ${thrift_file_abs})
    if (NOT EXISTS ${output_dir_abs})
      message(FATAL_ERROR "Error: Output directory ${output_dir_abs} does not exist for thrift generated files")
    endif ()
    message(STATUS "Generating sources using ${thrift_file_abs} to ${output_dir_abs}")
    exec_program(${THRIFT_COMPILER}
            ARGS -out "${output_dir_abs}" --gen ${lang} ${thrift_file_abs}
            OUTPUT_VARIABLE __thrift_OUT
            RETURN_VALUE THRIFT_RETURN)
  else ()
    message(FATAL_ERROR "Error: Cannot generate files from ${thrift_file_abs} since it does not exist")
  endif ()
endfunction()

# Generate cpp sources
thrift_gen(${THRIFT_GEN_CXX} src/block_registration_service.thrift ../src/metadata/block/)
thrift_gen(${THRIFT_GEN_CXX} src/inode_management_service.thrift ../src/metadata/blob/)
thrift_gen(${THRIFT_GEN_CXX} src/storage_management_service.thrift ../src/storage/)
thrift_gen(py src/model_service.thrift ../ml/)
thrift_gen(${THRIFT_GEN_CXX} src/model_service.thrift ../src/predictor/)
thrift_gen(${THRIFT_GEN_CXX} src/workflow_management_service.thrift ../src/workflow/)