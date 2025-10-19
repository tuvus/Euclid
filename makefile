.PHONY: help engine game run debug run-debug test clean

## Self-documenting makefile code thanks to https://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
help: ## Shows the available makefile targets
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

engine: ## Builds the engine
	$(MAKE) -C engine

game: ## Builds the game
	$(MAKE) -C game

debug: ## Builds the game with debug flags and address sanitizer
	$(MAKE) -C game debug

run: game ## Build and run the game
	$(MAKE) -C game run

run-debug: debug ## Build and run the game in debug mode
	$(MAKE) -C game run

test: ## Runs the GameEngine tests
	$(MAKE) -C engine test

clean: ## Cleans the build files from both the game engine and game
	$(MAKE) -C engine clean
	$(MAKE) -C game clean
