package must

import (
	"context"
	"errors"
)

func panicIfErr(err error) {
	if err != nil && err != context.Canceled && !errors.Is(err, context.Canceled) {
		panic(err)
	}
}

func ReturnNothing(err error) {
	panicIfErr(err)
}

func Return[T any](v T, err error) T {
	panicIfErr(err)
	return v
}

func Return2[T1 any, T2 any](v1 T1, v2 T2, err error) (T1, T2) {
	panicIfErr(err)
	return v1, v2
}

func Return3[T1 any, T2 any, T3 any](v1 T1, v2 T2, v3 T3, err error) (T1, T2, T3) {
	panicIfErr(err)
	return v1, v2, v3
}
