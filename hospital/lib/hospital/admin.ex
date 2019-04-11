defmodule Admin do
  @moduledoc """
  Implements admin/logger
"""

  use GenServer

  # Client

  def start(name) do
    GenServer.start_link(__MODULE__, %{name: name}, name: name)
  end

  def send(ref, message) do
    GenServer.cast(ref, {:info, message})
  end

  def stop(ref) do
    GenServer.stop(ref)
  end

  # Server (callbacks)

  def init(args) do

    # open connection
    {:ok, connection} = AMQP.Connection.open()
    {:ok, channel} = AMQP.Channel.open(connection)

    # create queues for receiving
    {:ok, _} = AMQP.Queue.declare(channel, "knee")
    {:ok, _} = AMQP.Queue.declare(channel, "hip")
    {:ok, _} = AMQP.Queue.declare(channel, "elbow")

    # create queue for logging
    {:ok, _} = AMQP.Queue.declare(channel, "log", auto_delete: true)

    # create exchange for sending to doctors
    AMQP.Exchange.declare(channel, "to_doctors", :topic)

    # bind logging queue to all exchanges
    AMQP.Queue.bind(channel, "log", "to_doctors", routing_key: "#")
    AMQP.Queue.bind(channel, "log", "to_technicians", routing_key: "#")
    # Not sure if this one is necessary
    # AMQP.Queue.bind(channel, "log", "to_all")

    # create exchange for sending
    AMQP.Exchange.declare(channel, "to_technicians", :topic)

    # bind queues for sending to exchange
    AMQP.Queue.bind(channel, "knee", "to_technicians",routing_key: "knee.#")
    AMQP.Queue.bind(channel, "elbow", "to_technicians",routing_key: "elbow.#")
    AMQP.Queue.bind(channel, "hip", "to_technicians", routing_key: "hip.#")

    # connect to logging queue
    {:ok, consumer_tag} = AMQP.Basic.consume(channel, "log", self())
    receive do
      {:basic_consume_ok, %{consumer_tag: ^consumer_tag}} ->
        :ok
    end

    # save parameters in state
    {:ok, %{conn: connection, chan: channel, name: args.name}}
  end


  def handle_info({:basic_deliver, message, meta}, state) do
    IO.puts("[Logger: #{state.name}]: #{inspect message}")
    AMQP.Basic.ack(state.chan, meta.delivery_tag)
    {:noreply, state}
  end

  def handle_cast({:info, message}, state) do
    IO.puts("[Admin: #{state.name}]: Sending \"#{message}\" to all.")
    AMQP.Basic.publish(state.chan,"to_all", "", "[Info]: "<>message)
    {:noreply, state}
  end

  def terminate(_reason, state) do
    {:ok, %{message_count: cnt}} = AMQP.Queue.delete(state.chan, "log")
    IO.puts("#{state.name}: Deleted log with #{cnt} messages. Bye!")
    :ok = AMQP.Channel.close(state.chan)
    :ok = AMQP.Connection.close(state.conn)
  end

end
