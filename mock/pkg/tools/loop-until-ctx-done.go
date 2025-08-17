package tools

import (
	"context"
	"iter"
)

func LoopUntilCtxDone(ctx context.Context) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		var i int64
		for {
			select {
			case <-ctx.Done():
				return
			default:
				if !yield(i) {
					return
				}
				i++
			}
		}
	}
}
