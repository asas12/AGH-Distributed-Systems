defmodule Doctor do
  @moduledoc """
  Implements doctors process.
"""
  use GenServer

  # Client

  def start(name) do
    GenServer.start_link(__MODULE__, name, name: name)
  end

  def stop(ref) do
    GenServer.stop(ref)
  end

  def send(ref, patient_name, task) do
    GenServer.cast(ref, {task, patient_name})
  end

  # Server (callbacks)

  def init(name) do

    # open connection
    {:ok, connection} = AMQP.Connection.open()
    {:ok, channel} = AMQP.Channel.open(connection)
    # create queues for sending
    {:ok, _} = AMQP.Queue.declare(channel, "knee")
    {:ok, _} = AMQP.Queue.declare(channel, "hip")
    {:ok, _} = AMQP.Queue.declare(channel, "elbow")
    # create queue for receiving
    {:ok, %{queue: my_queue}} = AMQP.Queue.declare(channel, "", auto_delete: true)

    # create exchange for receiving
    AMQP.Exchange.declare(channel, "to_doctors", :direct)

    # bind personal queue to exchange
    # as each doctor has his own queue, and it's name is unique, it will be used as routing key
    AMQP.Queue.bind(channel, my_queue, "to_doctors", routing_key: my_queue)

    # connect to receiving queue
    {:ok, consumer_tag} = AMQP.Basic.consume(channel, my_queue, self())
    receive do
      {:basic_consume_ok, %{consumer_tag: ^consumer_tag}} ->
        {:ok, consumer_tag}
    end

    # create exchange for sending
    AMQP.Exchange.declare(channel, "to_technicians", :topic)

    # bind queues for sending to exchange
    AMQP.Queue.bind(channel, "knee", "to_technicians",routing_key: "knee.#")
    AMQP.Queue.bind(channel, "elbow", "to_technicians",routing_key: "elbow.#")
    AMQP.Queue.bind(channel, "hip", "to_technicians", routing_key: "hip.#")


    # save parameters in state
    {:ok, %{conn: connection, chan: channel, queue: my_queue, tag: consumer_tag, name: name}}
  end

  def handle_cast({task, patient_name}, state) do
    IO.puts("Sending #{task} job, #{state.name} to #{state.queue}")
    s = Atom.to_string(task)<>"."<>state.queue
    IO.puts(s)
    AMQP.Basic.publish(state.chan,"to_technicians", Atom.to_string(task)<>"."<>state.queue, patient_name<>": "<>Atom.to_string(task))
    {:noreply, state}
  end


  def handle_info({:basic_deliver, message, meta}, state) do
    IO.puts("#{state.name}: Received message: #{inspect message}")
    AMQP.Basic.ack(state.chan, meta.delivery_tag)
    {:noreply, state}
  end

  def handle_info(_, state) do
    IO.puts("Processed info")
    {:noreply, state}
  end

  def handle_call(_,_, state) do
    IO.puts("Processed call")
    {:noreply, state}
  end

  def terminate(_reason, state) do
    {:ok, %{message_count: cnt}} = AMQP.Queue.delete(state.chan, state.queue)
    IO.puts("#{state.name}: Deleted the #{state.queue} with #{cnt} messages. Bye!")
    :ok = AMQP.Channel.close(state.chan)
    :ok = AMQP.Connection.close(state.conn)
  end


end


