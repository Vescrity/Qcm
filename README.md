# Qcm
Material3 cloud music player.  
Currently linux only.  

Music Service:  
- Jellyfin(wip)
- Netease Cloud Music


<table>
  <tr>
  <td><img src="https://github.com/hypengw/Qcm/blob/master/app/assets/screenshots/main.png?raw=true"></td>
  <td><img src="https://github.com/hypengw/Qcm/blob/master/app/assets/screenshots/main_compact.png?raw=true"></td>
  </tr>
  <tr>
  <td><img src="https://github.com/hypengw/Qcm/blob/master/app/assets/screenshots/main_dark.png?raw=true"></td>
  <td><img src="https://github.com/hypengw/Qcm/blob/master/app/assets/screenshots/main_compact_dark.png?raw=true"></td>
  </tr>
</table>

### Download:  
<a href='https://flathub.org/apps/details/io.github.hypengw.Qcm'><img width='240' alt='Download on Flathub' src='https://dl.flathub.org/assets/badges/flathub-badge-en.png'/></a>

### Require:  
- Qt 6.7 (quick, dbus)
- C++ 23
- Openssl 3
- Curl
- FFmpeg 6

### Build:  
```
git clone https://github.com/hypengw/Qcm.git  
git submodule update --init

cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release  
cmake --build build

# run without install
export QML_IMPORT_PATH=$PWD/build/qml_modules
./build/app/Qcm

# install
cmake --install build
```

### Todo:
- [ ] jellyfin
- [ ] sql api model
- [ ] subsonic
- [ ] android
- [ ] mac/win
- [ ] offline mode
- [x] user session switch
- [x] feedback
- [x] upload
- [x] fade in/out
- [x] sidebar popup
- [x] search page
- [x] lyric
- [x] audio cache using http proxy(AndroidVideoCache)
- [x] cache limit
- [x] sql cache
- [x] mpris
- [x] api
- [x] json
- [x] http lib(libcurl)

### Credits:
#### Libraries Used
- [Qt](https://www.qt.io/)
- [asio](https://github.com/chriskohlhoff/asio)
- [cubeb](https://github.com/mozilla/cubeb)
- [ffmpeg](https://www.ffmpeg.org/)
- [curl](https://curl.se/)
- [PEGTL](https://github.com/taocpp/PEGTL)
- [nlohmann/json](https://github.com/nlohmann/json)
