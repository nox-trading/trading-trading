package tools

import (
	"context"
	"iter"
	"time"
)

func PeriodicScheduler(ctx context.Context, duration time.Duration) iter.Seq[time.Time] {
	return func(yield func(time.Time) bool) {
		for {
			var toTime = time.Now().Truncate(duration).Add(duration)
			var timer = time.NewTimer(time.Until(toTime))
			select {
			case <-ctx.Done():
				return
			case timestamp := <-timer.C:
				if !yield(timestamp) {
					return
				}
			}
		}
	}
}
