package main

import (
	"context"
	"encoding/json"
	"fmt"
	"log/slog"
	"math"
	"math/rand"
	"mt-api-mock/pkg/must"
	"mt-api-mock/pkg/tools"
	"time"

	"github.com/nats-io/nats.go"
	"github.com/pkg/errors"
	"golang.org/x/sync/errgroup"
)

type OrderSide int

const (
	OrderSideBuy OrderSide = iota
	OrderSideSell
)

type TradeRequest struct {
	RequestID int       `json:"request_id"`
	Side      OrderSide `json:"side"`
	Login     int       `json:"login"`
	Volume    float64   `json:"volume"`
	SL        float64   `json:"sl"`
	TP        float64   `json:"tp"`
	Symbol    string    `json:"symbol"`
	Comment   string    `json:"comment"`
}

type TradeResponse struct {
	RequestID     int    `json:"request_id"`
	OrderID       int    `json:"order_id"`
	RejectCode    int    `json:"reject_code"`
	RejectMessage string `json:"reject_message"`
}

type GroupSymbol struct {
	AccountGroup string    `json:"account_group"`
	Symbol       string    `json:"symbol"`
	Description  string    `json:"description"`
	Digits       int       `json:"digits"`
	Mode         TradeMode `json:"mode"`
	ContractSize float64   `json:"contract_size"`
	TickSize     float64   `json:"tick_size"`
	SwapLong     float64   `json:"swap_long"`
	SwapShort    float64   `json:"swap_short"`
	LotMin       float64   `json:"lot_min"`
	LotMax       float64   `json:"lot_max"`
	LotStep      float64   `json:"lot_step"`
}

type TradeMode int

const (
	TradeNo TradeMode = iota
	TradeClose
	TradeFull
	TradeLongOnly
)

type Candle struct {
	Symbol string  `json:"symbol"`
	Ts     int32   `json:"ts"`
	Open   float64 `json:"open"`
	High   float64 `json:"high"`
	Low    float64 `json:"low"`
	Close  float64 `json:"close"`
}

type Tick struct {
	Symbol string  `json:"symbol"`
	Ts     int32   `json:"ts"`
	Bid    float64 `json:"bid"`
	Ask    float64 `json:"ask"`
}

func generateCandlesBySymbols(symbolList []string, ts time.Time) [][]byte {
	var candles [][]byte
	for _, symbol := range symbolList {
		price := math.Sin(float64(ts.Unix())) + 1.0
		candle := &Candle{
			Symbol: symbol,
			Ts:     int32(ts.Unix()),
			Open:   price - 0.1,
			High:   price + 0.1,
			Low:    price - 0.2,
			Close:  price + 0.05,
		}
		candles = append(candles,
			must.Return(json.Marshal(candle)),
		)
	}
	return candles
}

func publishCandles(nc *nats.Conn, topic string, symbolList []string, ts time.Time) error {
	for _, candle := range generateCandlesBySymbols(symbolList, ts) {
		if err := nc.Publish(topic, candle); err != nil {
			return errors.Wrap(err, "failed to publish candle data")
		}
	}
	return nil
}

func candlesGenerator(ctx context.Context, nc *nats.Conn, topic string, symbolList []string) error {
	// publish initial candles
	nowTime := time.Now().Truncate(time.Minute)
	for ts := range tools.TimeframeGenerator(nowTime.Add(-time.Minute*2), nowTime, time.Minute) {
		must.ReturnNothing(
			publishCandles(
				nc,
				topic,
				symbolList,
				ts,
			),
		)
	}

	for ts := range tools.PeriodicScheduler(ctx, time.Minute) {
		must.ReturnNothing(
			publishCandles(
				nc,
				topic,
				symbolList,
				ts,
			),
		)
	}
	return nil
}

type config struct {
	NatsUrl    string   `env:"NATS_URL,required"`
	ServerName string   `env:"SERVER_NAME,required"`
	SymbolList []string `env:"SYMBOL_LIST,required"`
	GroupNames []string `env:"GROUP_NAMES,required"`
}

func main() {
	slog.Info("Starting mt4 mock server")
	var ctx = tools.CreateGrexitContext(context.Background())

	cfg := must.Return(
		tools.ParseEnvConfig[config](),
	)

	nc := must.Return(
		nats.Connect(cfg.NatsUrl),
	)
	defer nc.Close()

	errGroup, ctx := errgroup.WithContext(ctx)

	// Publish candles
	errGroup.Go(func() error {
		return candlesGenerator(ctx, nc, fmt.Sprintf("%s.mt4_candle", cfg.ServerName), cfg.SymbolList)
	})

	// Publish ticks
	errGroup.Go(func() error {
		for ts := range tools.PeriodicScheduler(ctx, time.Millisecond*1000) {
			symbol := cfg.SymbolList[rand.Intn(len(cfg.SymbolList))]
			price := math.Sin(float64(ts.Unix())) + 1.0
			must.ReturnNothing(
				nc.Publish(
					fmt.Sprintf("%s.mt4_tick", cfg.ServerName),
					must.Return(json.Marshal(&Tick{
						Symbol: symbol,
						Ts:     int32(ts.Unix()),
						Bid:    price - 0.1,
						Ask:    price + 0.1,
					})),
				),
			)
		}
		return nil
	})

	errGroup.Go(func() error {
		sub, err := nc.SubscribeSync(
			fmt.Sprintf("%s.mt4_trade_request", cfg.ServerName),
		)
		if err != nil {
			return errors.Wrap(err, "failed to subscribe to updates")
		}

		var orderID int

		for range tools.LoopUntilCtxDone(ctx) {
			msg, err := sub.NextMsg(10 * time.Second)
			if err != nil {
				if err == nats.ErrTimeout {
					continue
				}
				return errors.Wrap(err, "failed to get next message")
			}

			var tradeRequest TradeRequest
			if err := json.Unmarshal(msg.Data, &tradeRequest); err != nil {
				slog.Error("Failed to unmarshal trade request", "error", err, "data", string(msg.Data))
			}

			orderID++

			var topic = fmt.Sprintf("%s.mt4_trade_response", cfg.ServerName)
			if rand.Intn(10) < 5 {
				must.ReturnNothing(
					nc.Publish(
						topic,
						must.Return(json.Marshal(
							&TradeResponse{
								RequestID:     tradeRequest.RequestID,
								OrderID:       orderID,
								RejectCode:    0,
								RejectMessage: "",
							},
						)),
					),
				)
			} else {
				must.ReturnNothing(
					nc.Publish(
						topic,
						must.Return(json.Marshal(
							&TradeResponse{
								RequestID:     tradeRequest.RequestID,
								OrderID:       0,
								RejectCode:    1,
								RejectMessage: "Wrong parameters",
							},
						)),
					),
				)
			}
		}

		return nil
	})

	// Publish group symbols
	for _, group := range cfg.GroupNames {
		for _, symbol := range cfg.SymbolList {
			must.ReturnNothing(
				nc.Publish(
					fmt.Sprintf("%s.mt4_group", cfg.ServerName),
					must.Return(json.Marshal(
						&GroupSymbol{
							AccountGroup: group,
							Symbol:       symbol,
							Description:  fmt.Sprintf("%s %s", group, symbol),
							Digits:       5,
							ContractSize: 100000,
							TickSize:     0.00001,
							SwapLong:     0.1,
							SwapShort:    0.1,
							LotMin:       0.01,
							LotMax:       10.0,
							LotStep:      0.01,
							Mode:         TradeFull,
						},
					)),
				),
			)
		}
	}

	if err := errGroup.Wait(); err != nil {
		slog.Error("Error occurred while publishing candles", "error", err)
	}
}
