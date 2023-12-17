#!/bin/bash

venv_name="encoderEnv"

if [ ! -d "$venv_name" ]; then
    echo "Creating virtual environment '$venv_name'..."
    python3 -m venv "$venv_name"

    echo "Activating virtual environment '$venv_name'..."
    source "$venv_name/bin/activate"

    if [ -f "requirements.txt" ]; then
        echo "Installing dependencies from requirements.txt..."
        pip install -r requirements.txt
    fi
fi
