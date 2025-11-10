import React, { useState, useEffect } from 'react'

const Orders = () => {
  const [tab, setTab] = useState('outgoing')
  const [items, setItems] = useState([])
  const [loading, setLoading] = useState(true)
  const maxLimit = 200; //max SKUs in one division
  const Priorities = ["", "Low", "Medium", "High"]

  const [restockOrders, setRestockOrders] = useState([])
  const [outgoingOrders, setOutgoingOrders] = useState([])

  const [restockForm, setRestockForm] = useState({ sku: '', qty: 0, priority: 1, etaDays: 7, date: "" })
  const [outgoingForm, setOutgoingForm] = useState({ sku: '', qty: 0, priority: 1, destination: '', date: "" })


  const loaditems = async () => {
    try {
      const res = await fetch('http://localhost:8080/inventory')
      if (!res.ok) throw new Error('Failed to load')
      const data = await res.json()

      setItems(data)
      setRestockForm({ sku: data[0]?.sku || '', qty: 0, priority: 1, etaDays: 7, date: "" })
      setOutgoingForm({ sku: data[0]?.sku || '', qty: 0, priority: 1, destination: '', date: "" })
    } catch (err) {
      console.error('Failed to load items for map:', err)
    }
  }
  const loadOrders = async () => {
    try {
      const res = await fetch('http://localhost:8080/orders')
      if (!res.ok) throw new Error('Failed to load')
      const data = await res.json()
      setOutgoingOrders(data || [])
    } catch (err) {
      console.error('Failed to load orders:', err)
    }
  }
  const loadReorders = async () => {
    try {
      const res = await fetch('http://localhost:8080/reorders')
      if (!res.ok) throw new Error('Failed to load')
      const data = await res.json()
      setRestockOrders(data || [])
    } catch (err) {
      console.error('Failed to load reorders:', err)
    }
  }

  useEffect(() => {
    setLoading(true)


    loaditems()
    loadOrders()
    loadReorders()
    setLoading(false)
  }, [])


  async function createOutgoing(order) {
    if (order.qty <= 0 || order.destination == "") return;
    order.date = (new Date()).toDateString()
    try {
      const res = await fetch('http://localhost:8080/orders', {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(order)
      })
      if (!res.ok) throw new Error('Failed to create outgoing order')
      const data = await res.text()
      loadOrders();
    } catch (err) {
      console.error('Failed to create outgoing order:', err)
    }

  }

  async function createRestock(reorder) {
    if (reorder.qty <= 0) return;
    reorder.date = (new Date()).toDateString()
    try {
      const res = await fetch('http://localhost:8080/reorders', {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(reorder)
      })
      if (!res.ok) throw new Error('Failed to create outgoing order')
      const data = await res.text()
      loadReorders();
    } catch (err) {
      console.error('Failed to create outgoing order:', err)
    }
  }

  async function restockOrder(approve) {
    try {
      const res = await fetch(`http://localhost:8080/reorders/${approve ? 'approve' : 'cancel'}`, {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json'
        }
      });
      if (!res.ok) throw new Error('Failed to restock order');
      const data = await res.json();
      setRestockOrders(prev => prev.slice(1));
    } catch (err) {
      console.error('Failed to restock order:', err);
    }
    if (approve) {
      loaditems()
    }
  }

  async function outgoingOrder(approve) {
    try {
      const res = await fetch(`http://localhost:8080/orders/${approve ? 'approve' : 'cancel'}`, {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json'
        }
      });
      if (!res.ok) throw new Error('Failed to restock order');
      const data = await res.json();
      setOutgoingOrders(prev => prev.slice(1));
    } catch (err) {
      console.error('Failed to restock order:', err);
    }
    if (approve) {
      loaditems()
    }
  }

  function skuLimit(currSku) {
    if (!currSku) return maxLimit;
    let count = items.filter(x => x.sku.split('-')[0] == currSku.split('-')[0]).reduce((acc, i) => acc + i.quantity, 0);
    return (count > 0) ? maxLimit - count : 0;
  }

  if (loading) return <div className="loader"></div>

  return (
    <div>
      <h2 className="text-4xl font-bold">Orders</h2>
      <p className="mt-2 text-gray-600">Create and track restock (purchase) orders and outgoing orders.</p>

      <div className="mt-6">
        <div className="flex gap-2">
          <button className={`px-3 py-1 rounded ${tab === 'outgoing' ? 'bg-blue-600 text-white' : 'bg-gray-200'}`} onClick={() => setTab('outgoing')}>Outgoing Orders</button>
          <button className={`px-3 py-1 rounded ${tab === 'restock' ? 'bg-blue-600 text-white' : 'bg-gray-200'}`} onClick={() => setTab('restock')}>Restock Orders</button>
          <button className="px-3 py-1 rounded bg-green-600 text-white" onClick={() => setTab('new-item')}>Add New Item</button>
        </div>

        {tab === 'outgoing' && (
          <div className="mt-4">
            <form onSubmit={(e) => { e.preventDefault(); createOutgoing(outgoingForm) }} className="flex justify-between items-end">
              <div>
                <label className="block text-sm">SKU</label>
                <select value={outgoingForm.sku} onChange={e => setOutgoingForm({ ...outgoingForm, sku: e.target.value })} className="p-2 border rounded">
                  {items.map(s => <option key={s.sku} value={s.sku}>{s.sku} — {s.name}</option>)}
                </select>
              </div>
              <div>
                <label className="block text-sm">Qty</label>
                <input type="number" value={outgoingForm.qty} min={1} max={items.find(item => item.sku === outgoingForm.sku)?.quantity || 0} onChange={e => setOutgoingForm({ ...outgoingForm, qty: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <label className="block text-sm">Priority</label>
                <select value={outgoingForm.priority} onChange={e => setOutgoingForm({ ...outgoingForm, priority: e.target.value })} className="p-2 border rounded">
                  {[1, 2, 3].map(i => <option key={i} value={i}>{Priorities[i]}</option>)}
                </select>
              </div>
              <div>
                <label className="block text-sm">Destination</label>
                <input required type="text" value={outgoingForm.destination} onChange={e => setOutgoingForm({ ...outgoingForm, destination: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <button className="bg-blue-600 hover:bg-blue-800 text-white px-4 py-2 rounded">Create Outgoing</button>
              </div>
            </form>

            <div className="mt-6">
              <h3 className="text-2xl font-semibold">Outgoing Orders</h3>
              {outgoingOrders.length === 0 ? (
                <p className="mt-2 text-gray-500">No outgoing orders yet.</p>
              ) : (
                <ul className="mt-3 space-y-2 overflow-scroll" style={{ maxHeight: '310px' }}>
                  {outgoingOrders.map((o, index) => (
                    <li key={index} className="p-3 bg-white rounded shadow-sm flex justify-between">
                      <div>
                        <div className="font-medium">{o.sku}</div>
                        <div className="text-sm text-gray-500">Qty: {o.qty} • Destination: {o.destination} • Priority: <span className="font-bold">{Priorities[o.priority]}</span></div>
                      </div>
                      <div className="text-right flex gap-3 items-end">
                        {index === 0 && <><button className="bg-green-500 hover:bg-green-700 text-white px-2 py-1 rounded" onClick={() => outgoingOrder(true)}>Approve</button>
                          <button className="bg-red-500 hover:bg-red-700 text-white px-2 py-1 rounded" onClick={() => outgoingOrder(false)}>Cancel</button></>}
                        <div className="text-sm text-gray-400">{o.date}</div>
                      </div>
                    </li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        )}

        {tab === 'restock' && (
          <div className="mt-4">
            <form onSubmit={(e) => { e.preventDefault(); createRestock(restockForm) }} className="flex justify-between items-end">
              <div>
                <label className="block text-sm">SKU</label>
                <select value={restockForm.sku} onChange={e => setRestockForm({ ...restockForm, sku: e.target.value })} className="p-2 border rounded">
                  {items.map(s => <option key={s.sku} value={s.sku}>{s.sku} — {s.name}</option>)}
                </select>
              </div>
              <div>
                <label className="block text-sm">Qty</label>
                <input type="number" value={restockForm.qty} min={0} max={skuLimit(restockForm.sku)} onChange={e => setRestockForm({ ...restockForm, qty: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <label className="block text-sm">Priority</label>
                <select value={restockForm.priority} onChange={e => setRestockForm({ ...restockForm, priority: e.target.value })} className="p-2 border rounded">
                  {[1, 2, 3].map(i => <option key={i} value={i}>{Priorities[i]}</option>)}
                </select>
              </div>
              <div>
                <label className="block text-sm">ETA (days)</label>
                <input required type="number" value={restockForm.etaDays} min={0} onChange={e => setRestockForm({ ...restockForm, etaDays: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <button className="bg-green-600 hover:bg-green-800 text-white px-4 py-2 rounded">Create PO</button>
              </div>
            </form>

            <div className="mt-6">
              <h3 className="text-2xl font-semibold">Restock Orders</h3>
              {restockOrders.length === 0 ? (
                <p className="mt-2 text-gray-500">No restock orders yet.</p>
              ) : (
                <ul className="mt-3 space-y-2 overflow-scroll" style={{ maxHeight: '310px' }}>
                  {restockOrders.map((o, index) => (
                    <li key={index} className="p-3 bg-white rounded shadow-sm flex justify-between">
                      <div>
                        <div className="font-medium">{o.sku}</div>
                        <div className="text-sm text-gray-500">Qty: {o.qty} • ETA: {o.eta} days • Priority: <span className="font-bold">{Priorities[o.priority]}</span></div>
                      </div>
                      <div className="text-right flex gap-3 items-end">
                        {index === 0 && <><button className="bg-green-500 hover:bg-green-700 text-white px-2 py-1 rounded" onClick={() => restockOrder(true)}>Approve</button>
                          <button className="bg-red-500 hover:bg-red-700 text-white px-2 py-1 rounded" onClick={() => restockOrder(false)}>Cancel</button></>}
                        <div className="text-sm text-gray-400">{o.date}</div>
                      </div>
                    </li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        )}

        {tab === 'new-item' && (
          <div className="mt-4 p-4 bg-yellow-50 border border-yellow-200 rounded">
            <h3 className="text-2xl font-semibold">Add New Inventory Item</h3>
            <p className="mt-2 text-gray-600">To add a new inventory item, please navigate to the Inventory page and use the "Add Item" button.</p>
          </div>
        )}
      </div>
    </div>
  )
}

export default Orders
