package tools

import (
	"os"

	"github.com/caarlos0/env"
	yaml "gopkg.in/yaml.v3"
)

func ParseEnvConfig[ConfigType any]() (*ConfigType, error) {
	var cfg = new(ConfigType)
	if err := env.Parse(cfg); err != nil {
		return nil, err
	}
	return cfg, nil
}

func ParseFileConfig[ConfigType any](path string) (*ConfigType, error) {
	yamlFile, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var appConfig = new(ConfigType)
	if err := yaml.Unmarshal(yamlFile, appConfig); err != nil {
		return nil, err
	}
	return appConfig, nil
}
