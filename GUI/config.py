"""
Configuration file for bot paths and game settings
"""

# Bot engine paths
bot_engines = {
    "Stockfish": "/opt/homebrew/bin/stockfish",
    "ChessEngine": "../chess_engine",  # Our custom engine
    "Bot2": ""  # Empty for bot vs bot second bot
}

# Current selected bots
selected_bot1 = "Stockfish"  # For 1 vs bot mode
selected_bot2 = ""  # For bot vs bot mode (empty by default)

def get_bot1_path():
    """Returns the path of the first bot"""
    return bot_engines.get(selected_bot1, bot_engines["Stockfish"])

def get_bot2_path():
    """Returns the path of the second bot"""
    return bot_engines.get(selected_bot2, "")

def set_bot1(bot_name):
    """Sets the first bot"""
    global selected_bot1
    if bot_name in bot_engines:
        selected_bot1 = bot_name

def set_bot2(bot_name):
    """Sets the second bot"""
    global selected_bot2
    if bot_name in bot_engines:
        selected_bot2 = bot_name