# TESMapMerge

This program was written to stitch together the map images generated from the CreateMaps command in Morrowind

## Installation
To combpile TESMapMerge you will need Visual Studio 10 or above.
Alternatively you can download a working executable from the Releases section on GitHub.

## Setup
To set up the tool first you need to generate map images for every cell in Morrowind.
Open your Morrowind.ini file 

Full instructions can be found here: http://en.uesp.net/wiki/Morrowind:Console and change the line that says 
```
Create Maps Enable=0
```
to
```
Create Maps Enable=2
```

Now run Morrowind and create a new game (you can load an existing save, but it takes longer to load generate a map for each cell if your save file is large).
As soon you can open the console by pressing ` and run the following command:
```
createmaps "Morrowind.esm"
```

Morrowid will now load every cell in the world and render a map of that cell as a 256x256 bitmap. This should take around 10-20 minutes, depending on the speed of your computer.
Once that is complete, you can close Morrowind.



Now inside the installation directory of Morrowind (for example, C:\Program Files (x86)\Bethesda Softworks\Morrowind) there now be a folder called maps filled with bitmap files.
You need to copy (or move) that maps folder to in the same directory as the directory where TESMapMerge is contained. For example:
```
.
├── Maps
|   ├── Ahemmusa Camp (11,16).bmp
|   ├── Ald Daedroth (11,20).bmp
|   └── ...
|
└── TESMapMerge
    ├── TESMapMerge.exe
    └── README.md
 ```
## Usage

After the setup has been complete, open a console in the directory where TESMapMerge.exe is installed and run
```
TESMapMerge.exe [filename]
```
Where [filename] is the name of the cell that should be used to fill out the blank areas of the map.
In my case I used "Wilderness (-21,28).bmp" because it matched the bottom of the sea bed quite closely, so I ran
```
TESMapMerge.exe "Wilderness (-21,28).bmp"
```
Now a file called output.bmp should be created with a complete mosaic of Vvardenfell, including Solstheim and Fort Firemoth if you have them installed.
