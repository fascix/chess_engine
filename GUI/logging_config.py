"""
Logging configuration for Pallas Chess Engine GUI
Provides centralized logging setup to replace print statements
"""

import logging
import sys
from pathlib import Path

# Create logs directory if it doesn't exist
LOGS_DIR = Path(__file__).parent.parent / "logs"
LOGS_DIR.mkdir(exist_ok=True)

def setup_logging(level=logging.INFO, log_to_file=True):
    """
    Configure logging for the application
    
    Args:
        level: Logging level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
        log_to_file: Whether to also log to a file
    """
    # Create formatters
    console_formatter = logging.Formatter(
        '%(levelname)s - %(name)s - %(message)s'
    )
    file_formatter = logging.Formatter(
        '%(asctime)s - %(levelname)s - %(name)s - %(message)s'
    )
    
    # Get root logger
    root_logger = logging.getLogger()
    root_logger.setLevel(level)
    
    # Clear any existing handlers
    root_logger.handlers.clear()
    
    # Console handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(level)
    console_handler.setFormatter(console_formatter)
    root_logger.addHandler(console_handler)
    
    # File handler (optional)
    if log_to_file:
        log_file = LOGS_DIR / "gui.log"
        file_handler = logging.FileHandler(log_file, mode='a')
        file_handler.setLevel(logging.DEBUG)  # Always log everything to file
        file_handler.setFormatter(file_formatter)
        root_logger.addHandler(file_handler)
    
    return root_logger

def get_logger(name):
    """
    Get a logger for a specific module
    
    Args:
        name: Name of the module (usually __name__)
    
    Returns:
        Logger instance
    """
    return logging.getLogger(name)

# Example usage:
# from logging_config import setup_logging, get_logger
# setup_logging(level=logging.DEBUG)
# logger = get_logger(__name__)
# logger.info("This is an info message")
# logger.debug("This is a debug message")
# logger.error("This is an error message")
