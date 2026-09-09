# Legacy PVCplus C toolkit — CMake build
#
# Replaces the original hand-rolled Makefiles (which hard-coded MacPorts paths,
# used macOS-only `ranlib -s`, had a broken cmusic_gen install step, and needed
# Dockerfile `sed` patching to build on Linux).
#
# Builds:
#   - libcarl   (static) : CARL cmusic support libs (libran/libdgl/libprocom/libfrm)
#   - gen tools          : cspline gen1..gen6 cannon  (control-function generators)
#   - libpvoc   (static) : the phase-vocoder DSP + I/O library
#   - 45 PVC tools       : plainpv, pvanalysis, ... (one executable per source)
#
# All executables install to ${CMAKE_INSTALL_BINDIR} (default: <prefix>/bin).
#
# Usage:
#   cmake -S legacy -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j
#   cmake --install build --prefix /opt/pvc-legacy

cmake_minimum_required(VERSION 3.16)
project(pvcplus_legacy C)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# 30-year-old K&R-ish C: multiple tentative global definitions across TUs need
# -fcommon (GCC 10+ defaults to -fno-common). -Wall for visibility; NOT -Werror.
add_compile_options(-fcommon -Wall -O2 -g)

# libsndfile (used by pvc_lib I/O)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

# ---------------------------------------------------------------------------
# libcarl  (cmusic_gen support libraries)
# Object lists mirror the original per-directory Makefiles exactly.
# Note: libran/gamma.c is intentionally excluded (commented out upstream);
# libdgl/crack.OLD.c is dead and excluded.
# ---------------------------------------------------------------------------
set(CARL_SRC
    # libran
    cmusic_gen/lib/libran/frand.c
    cmusic_gen/lib/libran/arcsin.c
    cmusic_gen/lib/libran/cauchy.c
    cmusic_gen/lib/libran/expn.c
    cmusic_gen/lib/libran/gauss.c
    cmusic_gen/lib/libran/hyper.c
    cmusic_gen/lib/libran/lin.c
    cmusic_gen/lib/libran/logist.c
    cmusic_gen/lib/libran/beta.c
    cmusic_gen/lib/libran/plapla.c
    cmusic_gen/lib/libran/onefrand.c
    cmusic_gen/lib/libran/corrand.c
    cmusic_gen/lib/libran/randfi.c
    cmusic_gen/lib/libran/randfc.c
    # libdgl
    cmusic_gen/lib/libdgl/crack.c
    cmusic_gen/lib/libdgl/floatsav.c
    # libprocom
    cmusic_gen/lib/libprocom/fgetfloat.c
    cmusic_gen/lib/libprocom/fputfloat.c
    cmusic_gen/lib/libprocom/getheader.c
    cmusic_gen/lib/libprocom/putheader.c
    cmusic_gen/lib/libprocom/putprop.c
    cmusic_gen/lib/libprocom/getprop.c
    cmusic_gen/lib/libprocom/finitbuf.c
    cmusic_gen/lib/libprocom/fflushfloat.c
    cmusic_gen/lib/libprocom/putfc.c
    cmusic_gen/lib/libprocom/cpioheader.c
    cmusic_gen/lib/libprocom/mkprop.c
    cmusic_gen/lib/libprocom/globals.c
    # libfrm
    cmusic_gen/lib/libfrm/expr.c
    cmusic_gen/lib/libfrm/polish.c
    cmusic_gen/lib/libfrm/trans.c
)
add_library(carl STATIC ${CARL_SRC})
target_include_directories(carl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cmusic_gen/include)

# gen control-function generators (exclude gen1x.c: not built upstream)
set(GEN_TOOLS cspline gen1 gen2 gen3 gen4 gen5 gen6 cannon)
foreach(g ${GEN_TOOLS})
  add_executable(${g} cmusic_gen/gen/${g}.c)
  target_include_directories(${g} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cmusic_gen/include)
  target_link_libraries(${g} PRIVATE carl m)
  install(TARGETS ${g} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

# ---------------------------------------------------------------------------
# libpvoc  (phase-vocoder DSP + I/O)
# Object list mirrors pvc_lib/Makefile BINARIES exactly (excludes *.OLD.c,
# *.old.c, *.TEST.c, and readin.c/openfile.c which were not in the Makefile).
# ---------------------------------------------------------------------------
set(PVOC_SRC
    pvc_lib/convert.c
    pvc_lib/convert_nounwrap.c
    pvc_lib/crack.c
    pvc_lib/fft.c
    pvc_lib/fold.c
    pvc_lib/leanconvert.c
    pvc_lib/leanunconvert.c
    pvc_lib/makewindows.c
    pvc_lib/noscbank.c
    pvc_lib/overlapadd.c
    pvc_lib/fileio.c
    pvc_lib/shiftin.c
    pvc_lib/shiftout.c
    pvc_lib/unconvert.c
    pvc_lib/getthresh.c
    pvc_lib/space.c
    pvc_lib/rands.c
    pvc_lib/stripheader.c
    pvc_lib/curve.c
    pvc_lib/fval.c
    pvc_lib/crackstring.c
    pvc_lib/crackstring_bin_only.c
    pvc_lib/crackstring_noprint.c
    pvc_lib/fillfunc.c
    pvc_lib/eq.c
    pvc_lib/eq2.c
    pvc_lib/spectmagwarp.c
    pvc_lib/spectmagwarp2.c
    pvc_lib/normalize.c
    pvc_lib/noscbank2.c
    pvc_lib/noscbank3.c
    pvc_lib/compress.c
    pvc_lib/pr.c
    pvc_lib/smoothspec.c
    pvc_lib/tprintspec.c
    pvc_lib/RIfindpeak.c
    pvc_lib/noscbank_extra.c
    pvc_lib/invertresponse.c
    pvc_lib/miscellania.c
    pvc_lib/tprintspec_groupdelay.c
    pvc_lib/smooth.c
    pvc_lib/CartesianSmooth.c
    pvc_lib/smoothfreqs.c
    pvc_lib/smoothnew.c
    pvc_lib/smooth_amp_change.c
    pvc_lib/smooth_zero.c
    pvc_lib/find_peak_amp.c
    pvc_lib/compand.c
    pvc_lib/make_freqresponse.c
    pvc_lib/threshold_limit.c
    pvc_lib/spectmagwarp_inharm.c
    pvc_lib/find_centroid.c
    pvc_lib/find_fluxoid.c
    pvc_lib/cut_data_lines.c
    pvc_lib/eq3.c
    pvc_lib/eq4.c
    pvc_lib/AnalyzePartials.c
    pvc_lib/fixTildeInFilename.c
    pvc_lib/FileTest_ASCIIorBinaryFloat.c
    pvc_lib/findFreqOfPeakFormant.c
    pvc_lib/funcStats.c
    pvc_lib/makeInterpolatedFilterFrame.c
    pvc_lib/makeFilterFrame.c
    pvc_lib/getFormants.c
    pvc_lib/writeSpectrumPlotFile.c
    pvc_lib/NormalizeToPeaksOfSpectrumInBand.c
    pvc_lib/findFilterTimeAndConstrainByWindow.c
    pvc_lib/normalizeLoopAmplitudes.c
    pvc_lib/makeLoopSmoothTime.c
    pvc_lib/cutDC.c
    pvc_lib/OPPC_to_Hz.c
    pvc_lib/Hz_to_OPPC.c
    pvc_lib/windows.c
    pvc_lib/findFuncMinMaxAvg.c
    pvc_lib/getGlobalFunctionValues.c
    pvc_lib/setupToneBankOrPartialFunctionValues.c
    pvc_lib/crackfloat.c
    pvc_lib/autoplay.c
    pvc_lib/filesToRemove.c
    pvc_lib/printUnixCommandLine.c
    pvc_lib/findArrayStats.c
    pvc_lib/findFuncStats.c
    pvc_lib/Hz_to_MIDI.c
    pvc_lib/findMode.c
    pvc_lib/sortArray.c
    pvc_lib/phaselock.c
    pvc_lib/find_spectralflatness.c
)
add_library(pvoc STATIC ${PVOC_SRC})
target_include_directories(pvoc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/pvc_lib
    ${SNDFILE_INCLUDE_DIRS})
target_link_libraries(pvoc PUBLIC ${SNDFILE_LIBRARIES} m)
target_link_directories(pvoc PUBLIC ${SNDFILE_LIBRARY_DIRS})

# ---------------------------------------------------------------------------
# PVC tools — one executable per source in pvc_src (MOLES).
# ---------------------------------------------------------------------------
set(PVC_TOOLS
    plainpv readheader pvanalysis groupdelaymaker reshape
    envelope centroid fluxoid freqresponse filtresponsemaker
    chordresponsemaker peakformant filter ring twarp
    ringfilter ringtvfilter spectwarper compander tvfilter
    filtdeviator spectralextractor chordmapperplus noisefilter
    amptodB dBtoamp Hztopitch pitchtoHz tvfiltdeviator
    formantsmapper harmonizer inharmonator delayfilter pitchtracker
    convolver impulseresponse irconvolver irconvolvesequencer
    channelmix channelcollect mixfiles ratechanger specflattracker
    spectrummapper
    roomresponsemaker roomresponsesequencer
)
foreach(t ${PVC_TOOLS})
  add_executable(${t} pvc_src/${t}.c)
  target_link_libraries(${t} PRIVATE pvoc)
  install(TARGETS ${t} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

# Convenience: report tool count at configure time.
list(LENGTH PVC_TOOLS _ntools)
list(LENGTH GEN_TOOLS _ngen)
message(STATUS "pvcplus_legacy: ${_ntools} PVC tools + ${_ngen} gen tools")
