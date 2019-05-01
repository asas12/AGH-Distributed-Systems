defmodule Technician do
  @moduledoc """
  Implements technicians process.
"""
  use GenServer

  # Client

  def start(name, skill_one, skill_two) do
    GenServer.start_link(__MODULE__, %{ name: name, skills: [skill_one, skill_two]}, name: name)
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

    # set prefetch count to one
    AMQP.Basic.qos(channel, prefetch_count: 1)

    # create queue for receiving from admin
    {:ok, %{queue: my_queue}} = AMQP.Queue.declare(channel, "", auto_delete: true)

    # create exchange for sending to doctors
    AMQP.Exchange.declare(channel, "to_doctors", :topic)

    # create exchange for sending info from admin
    AMQP.Exchange.declare(channel, "to_all", :fanout)

    # bind personal queue to admin exchange
     AMQP.Queue.bind(channel, my_queue, "to_all")

    # create exchange for sending
    AMQP.Exchange.declare(channel, "to_technicians", :topic)

    # bind queues for sending to exchange
    AMQP.Queue.bind(channel, "knee", "to_technicians",routing_key: "knee.#")
    AMQP.Queue.bind(channel, "elbow", "to_technicians",routing_key: "elbow.#")
    AMQP.Queue.bind(channel, "hip", "to_technicians", routing_key: "hip.#")

    # connect to receiving queues
    for skill <- args.skills do
      skill = skill |> to_string
      {:ok, consumer_tag} = AMQP.Basic.consume(channel, skill, self())
      receive do
        {:basic_consume_ok, %{consumer_tag: ^consumer_tag}} ->
          :ok
      end
    end

    # connect to queue from admin
    {:ok, consumer_tag} = AMQP.Basic.consume(channel, my_queue, self())
    receive do
      {:basic_consume_ok, %{consumer_tag: ^consumer_tag}} ->
        {:ok, consumer_tag}
    end

    # save parameters in state
    {:ok, %{conn: connection, chan: channel, queue: my_queue, name: args.name}}
  end


  def handle_info({:basic_deliver, message, meta}, state) do
    if message |> String.slice(0..5) == "[Info]" do
      IO.puts("#{state.name}: Got message: #{inspect message}")
      AMQP.Basic.ack(state.chan, meta.delivery_tag)
    else

        # simulate work
        Process.sleep(5000)
        IO.puts("#{state.name}: Processed message: #{inspect message}")
        AMQP.Basic.ack(state.chan, meta.delivery_tag)

        # get queueID
        [_task, qID] = meta.routing_key |> String.split(".", parts: 2)

        # send reply
        AMQP.Basic.publish(state.chan, "to_doctors", qID , message<>" - done.")

    end

    {:noreply, state}
  end

  def terminate(_reason, state) do
    {:ok, %{message_count: cnt}} = AMQP.Queue.delete(state.chan, state.queue)
    IO.puts("#{state.name}: Deleted the #{state.queue} with #{cnt} messages. Bye!")
    :ok = AMQP.Channel.close(state.chan)
    :ok = AMQP.Connection.close(state.conn)
  end

end
