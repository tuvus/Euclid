.PHONY: help


## Self-documenting makefile code thanks to https://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
help: ## Shows the available makefile targets
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

buildengine: ## builds the engine
	cd engine && make && cd ..

buildgame: ## Builds the game
	cd game && make && cd ..

run: buildgame ## build and run the game
	cd game && make run && cd ..

debug: ## Builds the game with debug flags and address sanitizer
	cd game && make debug && cd ..

run-debug: debug ## build and run the game in debug mode
	cd game && make run && cd ..

test: ## Runs the GameEngine tests
	cd engine && make test && cd ..

clean: ## Cleans the build files from both the game engine and game
	cd engine && make clean && cd ../game && make clean && cd ..
