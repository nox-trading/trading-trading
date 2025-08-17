package tools

import "github.com/rs/xid"

func GenerateUID() string {
	return xid.New().String()
}
