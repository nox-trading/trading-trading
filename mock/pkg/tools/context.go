package tools

import "context"

func ToContext[KeyType any, ValueType any](ctx context.Context, key KeyType, value ValueType) context.Context {
	return context.WithValue(ctx, key, value)
}

func FromContext[KeyType any, ValueType any](ctx context.Context, key KeyType) ValueType {
	if value, valid := ctx.Value(key).(ValueType); valid {
		return value
	}
	var none ValueType
	return none
}
