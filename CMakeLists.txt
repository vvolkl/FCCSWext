CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)


#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
find_package(FCCSW)
#---------------------------------------------------------------

gaudi_project(FCCSWext HEAD
              USE FCCSW head 
              USE Gaudi v29r2 )
