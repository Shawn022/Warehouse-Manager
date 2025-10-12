import React, { useState } from 'react'

// Simple in-memory mock orders UI. Replace persistence with API calls as needed.
const Orders = () => {
  const [tab, setTab] = useState('restock')

  // mock SKU options - in a real app this would come from inventory
  const skuOptions = [
    { sku: 'A-100', name: 'Widget A' },
    { sku: 'B-200', name: 'Widget B' },
    { sku: 'C-300', name: 'Gadget C' },
  ]

  const [restockOrders, setRestockOrders] = useState([])
  const [outgoingOrders, setOutgoingOrders] = useState([])

  const [restockForm, setRestockForm] = useState({ sku: skuOptions[0].sku, qty: 50, etaDays: 7 })
  const [outgoingForm, setOutgoingForm] = useState({ sku: skuOptions[0].sku, qty: 10, destination: '' })

  function createRestock(e) {
    e.preventDefault()
    const order = {
      id: `PO-${Date.now()}`,
      sku: restockForm.sku,
      qty: Number(restockForm.qty),
      etaDays: Number(restockForm.etaDays),
      status: 'Pending',
      createdAt: new Date().toISOString(),
    }
    setRestockOrders(prev => [order, ...prev])
    setRestockForm({ ...restockForm, qty: 50 })
  }

  function createOutgoing(e) {
    e.preventDefault()
    const order = {
      id: `SO-${Date.now()}`,
      sku: outgoingForm.sku,
      qty: Number(outgoingForm.qty),
      destination: outgoingForm.destination,
      status: 'Created',
      createdAt: new Date().toISOString(),
    }
    setOutgoingOrders(prev => [order, ...prev])
    setOutgoingForm({ ...outgoingForm, qty: 10, destination: '' })
  }

  return (
    <div>
      <h2 className="text-4xl font-bold">Orders</h2>
      <p className="mt-2 text-gray-600">Create and track restock (purchase) orders and outgoing orders.</p>

      <div className="mt-6">
        <div className="flex gap-2">
          <button className={`px-3 py-1 rounded ${tab === 'restock' ? 'bg-blue-600 text-white' : 'bg-gray-200'}`} onClick={() => setTab('restock')}>Restock Orders</button>
          <button className={`px-3 py-1 rounded ${tab === 'outgoing' ? 'bg-blue-600 text-white' : 'bg-gray-200'}`} onClick={() => setTab('outgoing')}>Outgoing Orders</button>
          <button className="px-3 py-1 rounded bg-green-600 text-white" onClick={() => setTab('new-item')}>Add New Item</button>
        </div>

        {tab === 'restock' && (
          <div className="mt-4">
            <form onSubmit={createRestock} className="grid grid-cols-1 md:grid-cols-4 gap-3 items-end">
              <div>
                <label className="block text-sm">SKU</label>
                <select value={restockForm.sku} onChange={e => setRestockForm({ ...restockForm, sku: e.target.value })} className="p-2 border rounded">
                  {skuOptions.map(s => <option key={s.sku} value={s.sku}>{s.sku} — {s.name}</option>)}
                </select>
              </div>
              <div>
                <label className="block text-sm">Qty</label>
                <input type="number" value={restockForm.qty} min={1} onChange={e => setRestockForm({ ...restockForm, qty: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <label className="block text-sm">ETA (days)</label>
                <input type="number" value={restockForm.etaDays} min={0} onChange={e => setRestockForm({ ...restockForm, etaDays: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <button className="bg-green-600 text-white px-4 py-2 rounded">Create PO</button>
              </div>
            </form>

            <div className="mt-6">
              <h3 className="text-2xl font-semibold">Recent Restock Orders</h3>
              {restockOrders.length === 0 ? (
                <p className="mt-2 text-gray-500">No restock orders yet.</p>
              ) : (
                <ul className="mt-3 space-y-2">
                  {restockOrders.map(o => (
                    <li key={o.id} className="p-3 bg-white rounded shadow-sm flex justify-between">
                      <div>
                        <div className="font-medium">{o.id} — {o.sku}</div>
                        <div className="text-sm text-gray-500">Qty: {o.qty} • ETA: {o.etaDays} days • Status: {o.status}</div>
                      </div>
                      <div className="text-sm text-gray-400">{new Date(o.createdAt).toLocaleString()}</div>
                    </li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        ) } 
        {tab==='outgoing' && (
          <div className="mt-4">
            <form onSubmit={createOutgoing} className="grid grid-cols-1 md:grid-cols-4 gap-3 items-end">
              <div>
                <label className="block text-sm">SKU</label>
                <select value={outgoingForm.sku} onChange={e => setOutgoingForm({ ...outgoingForm, sku: e.target.value })} className="p-2 border rounded">
                  {skuOptions.map(s => <option key={s.sku} value={s.sku}>{s.sku} — {s.name}</option>)}
                </select>
              </div>
              <div>
                <label className="block text-sm">Qty</label>
                <input type="number" value={outgoingForm.qty} min={1} onChange={e => setOutgoingForm({ ...outgoingForm, qty: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <label className="block text-sm">Destination</label>
                <input type="text" value={outgoingForm.destination} onChange={e => setOutgoingForm({ ...outgoingForm, destination: e.target.value })} className="p-2 border rounded" />
              </div>
              <div>
                <button className="bg-blue-600 text-white px-4 py-2 rounded">Create Outgoing</button>
              </div>
            </form>

            <div className="mt-6">
              <h3 className="text-2xl font-semibold">Recent Outgoing Orders</h3>
              {outgoingOrders.length === 0 ? (
                <p className="mt-2 text-gray-500">No outgoing orders yet.</p>
              ) : (
                <ul className="mt-3 space-y-2">
                  {outgoingOrders.map(o => (
                    <li key={o.id} className="p-3 bg-white rounded shadow-sm flex justify-between">
                      <div>
                        <div className="font-medium">{o.id} — {o.sku}</div>
                        <div className="text-sm text-gray-500">Qty: {o.qty} • Destination: {o.destination} • Status: {o.status}</div>
                      </div>
                      <div className="text-sm text-gray-400">{new Date(o.createdAt).toLocaleString()}</div>
                    </li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        )}
        {tab==='new-item' && (
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
