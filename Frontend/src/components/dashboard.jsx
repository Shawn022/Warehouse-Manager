import React, { useState, useEffect, useMemo } from 'react'
import { Link } from 'react-router-dom'

const smallCard = (title, value, note) => (
  <div className="p-4 bg-white rounded-lg shadow-sm">
    <div className="text-sm text-gray-500">{title}</div>
    <div className="text-2xl font-bold">{value}</div>
    {note && <div className="text-xs text-gray-400">{note}</div>}
  </div>
)

  const Priorities = ["", "Low", "Medium", "High"]


const Dashboard = () => {
  const [inventory, setInventory] = useState([])
  const [loading, setLoading] = useState(true)

  const [restockOrders, setRestockOrders] = useState([])
  const [outgoingOrders, setOutgoingOrders] = useState([])

  useEffect(() => {
    setLoading(true)

    loadinventory()
    loadOrders()
    loadReorders()

    setLoading(false)
  }, [])

  const loadinventory = async () => {
  try {
    const res = await fetch('http://localhost:8080/inventory')
    if (!res.ok) throw new Error('Failed to load')
    const data = await res.json()

    setInventory(data)
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


  const kpis = useMemo(() => {
    const totalSKUs = inventory.length
    const totalUnits = inventory.reduce((s, i) => s + (i.quantity || 0), 0)
    const lowStock = inventory.filter(i => (i.quantity ?? 0) <= (i.reorder ?? 0)).length
    const pendingPOs = restockOrders.length
    return { totalSKUs, totalUnits, lowStock, pendingPOs }
  }, [inventory, restockOrders])
 
  if(loading) return <div className='loader'></div> 

  return (
    <div>

      <div className="flex items-center justify-between">
        <h1 className="text-4xl font-bold">Dashboard</h1>
        <div className="text-sm text-gray-500">Overview</div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-4 gap-4 mt-6">
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Total SKUs</div>
          <div className="text-2xl font-bold">{kpis.totalSKUs}</div>
        </div>
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Total Units</div>
          <div className="text-2xl font-bold">{kpis.totalUnits}</div>
        </div>
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Low Stock</div>
          <div className="text-2xl font-bold">{kpis.lowStock}</div>
        </div>
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Pending POs</div>
          <div className="text-2xl font-bold">{kpis.pendingPOs}</div>
        </div>
      </div>

      <div className="mt-8 grid grid-cols-1 md:grid-cols-3 gap-6">
        <div className="col-span-2 bg-white p-4 rounded shadow-sm">
          <h3 className="text-xl font-semibold">Recent Restock Orders</h3>
          {restockOrders.length === 0 ? (
            <p className="mt-2 text-gray-500">No Restock orders.</p>
          ) : (
            <ul className="mt-3 space-y-2">
              {restockOrders.slice(0, 6).map((o,index) => (
                <li key={index} className="p-3 border rounded flex justify-between">
                  <div>
                    <div className="font-medium">{inventory.find(i => i.sku === o.sku).name} • {o.sku}</div>
                    <div className="text-sm text-gray-500">{o.type} • Qty: {o.qty} •Priority <span className="font-bold">{Priorities[o.priority]}</span></div>
                  </div>
                  <div className="text-sm text-gray-400">{o.date}</div>
                </li>
              ))}
            </ul>
          )}
        </div>

        <div className="bg-white p-4 rounded shadow-sm">
          <h3 className="text-xl font-semibold">Warehouse Trends</h3>
          <div className="mt-3">
            {/* Simple aggregated incoming vs outgoing chart (text + bars) */}
            <div className="text-sm text-gray-500">Incoming vs Outgoing </div>
            <div className="mt-2">
              {/* build sample aggregates from orders */}
              {(() => {
                const incoming = restockOrders.reduce((s, o) => s + (o.qty || 0), 0)
                const outgoing = outgoingOrders.reduce((s, o) => s + (o.qty || 0), 0)
                const max = Math.max(1, incoming, outgoing)
                const incPct = Math.round((incoming / max) * 100)
                const outPct = Math.round((outgoing / max) * 100)
                return (
                  <div>
                    <div className="mt-2">
                      <div className="flex items-center justify-between text-xs text-gray-600"><span>Incoming</span><span>{incoming}</span></div>
                      <div className="h-3 bg-gray-100 rounded mt-1 overflow-hidden">
                        <div style={{ width: `${incPct}%` }} className="h-full bg-green-400" />
                      </div>
                    </div>
                    <div className="mt-3">
                      <div className="flex items-center justify-between text-xs text-gray-600"><span>Outgoing</span><span>{outgoing}</span></div>
                      <div className="h-3 bg-gray-100 rounded mt-1 overflow-hidden">
                        <div style={{ width: `${outPct}%` }} className="h-full bg-red-400" />
                      </div>
                    </div>
                  </div>
                )
              })()}
            </div>

            <div className="mt-4">
              <div className="text-sm text-gray-500">Top low-stock SKUs</div>
              {(() => {
                const low = inventory
                  .filter(i => (i.quantity ?? 0) <= (i.reorder ?? 0))
                  .sort((a, b) => a.quantity - b.quantity)
                  .slice(0, 5)
                if (low.length === 0) return <div className="mt-2 text-gray-500">All SKUs healthy</div>
                return (
                  <ul className="mt-2 space-y-1 text-sm">
                    {low.map(i => (
                      <li key={i.sku} className="flex justify-between"><span>{i.sku} • {i.name}</span><span className="text-gray-600">{i.quantity}</span></li>
                    ))}
                  </ul>
                )
              })()}
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

export default Dashboard




