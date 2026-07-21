del "./symbols_GBD_GB.png"
del "./symbols_NIGHT_GB.png"
del "./symbols_DAY_GB.png"
del "./symbols_DUSK_GB.png"
del chartsymbols.xml

s52symbol ./PresLib_e4.0.0GB.dai ./cie2rgb.ini ./
ren "./chartsymbols40_GB.xml" "./chartsymbols.xml"

copy /y "./symbols_WIN_PPT.png" "./symbols_GBD_GB.png"
