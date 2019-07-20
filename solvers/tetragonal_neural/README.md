Tetragonal Neural Solver
===
Solver for working with "tetragonal" field: cells have square form, number into cell shows amount of mines in 8 nearest cells.   
Main goal of solver is create trained model and store into binary format for usage by bot.

### Components:
**mines_field_generator.py** - python script for test field generation. Can generate pack of fields.  
**miner_dnn.py** - class for model training, exporting, etc.  
**data/minesweeper-bot/model.zip** - pre-trained model. See miner_dnn.py for usage.
**packer** - C++ packer of text exported model into binary form for usage by bot.  

### Usage:
**mines_field_generator.py**  
Generate number of test fields for model training, testing, etc.  
You need specify:  
row - field row amount;  
col - field columns amount;  
mines - mines amount into field;  
file prefix - specify file name in format <prefix>_<index>.txt  
start index - index in file name of first created file  
files amount - amount of created (generated) files


**miner_dnn.py**  
The class is used with emulator. See ../../emulator/README.md.
It can train or new model either pre-trained model. For model usage extract files from data/minesweeper-bot/model.zip at same folder.
The class can "play" minesweeper game.
Also it can export model data as .txt files. These files are used for packing into binary format.



### Notes:
Used versions of Python, C++, etc are described in common README.md file (../../README.md).
