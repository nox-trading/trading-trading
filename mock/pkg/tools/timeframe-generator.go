package tools

import (
	"iter"
	"time"
)

func TimeframeGenerator(from time.Time, to time.Time, duration time.Duration) iter.Seq[time.Time] {
	from = from.Truncate(duration)
	to = to.Truncate(duration)
	return func(yield func(time.Time) bool) {
		for !from.IsZero() && from.Before(to) {
			from = from.Add(duration)
			if !yield(from) {
				break
			}
		}
	}
}
