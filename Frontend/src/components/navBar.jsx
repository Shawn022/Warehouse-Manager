import './navbar.css'
import warehouse from '../assets/warehouse.svg'
import { NavLink } from 'react-router-dom'

const Navbar = () => {
  return (
    <div className='navbar'>
      <div className="heading flex items-center mb-5 gap-2 mt-3">
        <img src={warehouse} alt="Warehouse logo" style={{ display: 'inline-block', verticalAlign: 'middle', marginRight: '8px' }} />
        <h1 className='text-2xl font-bold mb-4'>WareHouse & Logistics Manager</h1>

      </div>
      <ul>
        <NavLink to="/"><li className='text-lg font-semibold'>Dashboard</li></NavLink>
        <NavLink to="/orders"><li className='text-lg font-semibold'>Orders</li></NavLink>
        <NavLink to="/inventory"><li className='text-lg font-semibold'>Inventory</li></NavLink>
        <NavLink to="/logistics"><li className='text-lg font-semibold'>Logistics</li></NavLink>
        <NavLink to="/map"><li className='text-lg font-semibold'>Map</li></NavLink>
      </ul>
    </div>
  )
}

export default Navbar

