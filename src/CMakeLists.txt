add_subdirectory(lisp)
add_subdirectory(net)
add_subdirectory(imlib)
add_subdirectory(sdlport)

add_executable(abuse
  common.h
  lol/matrix.cpp
  lol/matrix.h
  lol/timer.cpp
  lol/timer.h
  specache.cpp
  specache.h
  netcfg.cpp
  netcfg.h
  innet.cpp
  chat.cpp
  chat.h
  endgame.cpp
  loadgame.cpp
  loadgame.h
  profile.cpp
  profile.h
  cop.cpp
  cop.h
  statbar.cpp
  compiled.cpp
  compiled.h
  ant.cpp
  ant.h
  sensor.cpp
  demo.cpp
  demo.h
  lcache.cpp
  lcache.h
  nfclient.cpp
  nfclient.h
  clisp.cpp
  clisp.h
  gui.cpp
  gui.h
  transp.cpp
  transp.h
  collide.cpp
  property.cpp
  property.h
  cache.cpp
  cache.h
  particle.cpp
  particle.h
  objects.cpp
  objects.h
  extend.cpp
  extend.h
  console.cpp
  console.h
  ability.cpp
  ability.h
  items.cpp
  items.h
  dev.cpp
  dev.h
  chars.cpp
  chars.h
  level.cpp
  level.h
  smallfnt.cpp
  automap.cpp
  automap.h
  help.cpp
  help.h
  intsect.cpp
  intsect.h
  loader2.cpp
  loader2.h
  seq.cpp
  seq.h
  points.cpp
  points.h
  fnt6x13.cpp
  morpher.cpp
  morpher.h
  menu.cpp
  menu.h
  director.cpp
  director.h
  view.cpp
  view.h
  configuration.cpp
  configuration.h
  game.cpp
  game.h
  light.cpp
  light.h
  devsel.cpp
  devsel.h
  crc.cpp
  crc.h
  gamma.cpp
  gamma.h
  id.h
  netface.h
  isllist.h
  sbar.h
  nfserver.h
  ui/volumewindow.cpp
  ui/volumewindow.h
)

target_link_libraries(abuse lisp)
target_link_libraries(abuse sdlport)
target_link_libraries(abuse imlib)
target_link_libraries(abuse net)
target_link_libraries(abuse ${SDL2_MIXER_LIBRARIES})
target_link_libraries(abuse SDL2::SDL2)
target_link_libraries(abuse z FLAC vorbisfile vorbis ogg mikmod mpg123 modplug)
target_link_libraries(abuse -Wl,--whole-archive pthread -Wl,--no-whole-archive)

include_directories(
    ${abuse_SOURCE_DIR}/src
    ${abuse_SOURCE_DIR}/src/lisp
    ${abuse_SOURCE_DIR}/src/imlib
    ${abuse_SOURCE_DIR}/src/net
    ${SDL2_INCLUDE_DIR}
    ${SDL2_MIXER_INCLUDE_DIR}
)

vita_create_self(${PROJECT_NAME}.self abuse UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ../sce_sys sce_sys
    FILE ../data data
)
